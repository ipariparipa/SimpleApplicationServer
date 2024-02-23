/*
This file is part of sasODBC.

sasODBC is free software: you can redistribute it and/or modify
it under the terms of the Lesser GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

sasODBC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with sasODBC.  If not, see <http://www.gnu.org/licenses/>
*/

#include "odbcconnector.h"

#include "include_odbc.h"

#include <sasCore/application.h>
#include <sasCore/configreader.h>
#include <sasSQL/errorcodes.h>
#include <sasSQL/sqlstatement.h>
#include <sasSQL/sqldatetime.h>
#include <sasSQL/sqlresult.h>

#include "odbcstatement.h"
#include "odbctools.h"
#include <sasCore/thread.h>
#include <sasCore/tools.h>

#include <vector>
#include <list>
#include <mutex>
#include <memory>
#include <assert.h>
#include <string.h>
#include <regex>

#include <list>

namespace SAS {

struct ODBCConnector::Priv
{
    Priv(SQLHENV env_, Application * app_, const std::string & name_) :
		env(env_),
		app(app_),
		name(name_),
		logger(Logging::getLogger("SAS.OraConnector." + name_)),
		connectionManager(settings, connection_data, name_)
	{ }

	SQLHENV env;
	Application * app;
	std::string name;
	Logging::LoggerPtr logger;
	ODBC_Settings settings;

	struct ConnectionData
	{
		std::string dsn;
		std::string user;
		std::string passwd;
		std::list<std::pair<std::string, std::string>> options;
	} connection_data;
	std::mutex connection_data_mut;

	struct ConnectionManager //: public TimerThread
	{
		const ODBC_Settings & settings;
		const ConnectionData & connection_data;
		Logging::LoggerPtr logger;
		SQLHENV env = NULL;
		SQLHDBC conn = NULL;

		ConnectionManager(const ODBC_Settings & settings_, const ConnectionData & connection_data_, const std::string & name) :
			settings(settings_),
			connection_data(connection_data_),
			logger(Logging::getLogger("SAS.OraConnector." + name + ".ConnectionManager."))
		{ }

		bool init(SQLHENV env_, ErrorCollector & ec)
		{
            (void)ec;
			SAS_LOG_NDC();
			env = env_;

			return true;
		}

		struct Connection
		{
			Connection(SQLHENV env_, SQLHDBC conn_) : env(env_), conn(conn_)
			{ }

			~Connection()
			{
				std::unique_lock<std::mutex> __locker(mut);
				SQLFreeConnect(conn);
			}

			SQLHENV env;
			SQLHDBC conn;
			std::mutex mut;
			std::mutex external_mut;
			bool connected = false;
		};


		std::mutex connection_repo_mut;
		std::map<ThreadId, Connection*> connection_registry;
		std::map<Connection*, size_t> connection_repo;

		Connection * connection(ErrorCollector & ec)
		{
			SAS_LOG_NDC();

			std::unique_lock<std::mutex> __locker(connection_repo_mut);
			auto & conn = connection_registry[Thread::getThreadId()];
			if (!conn)
			{
				if (settings.max_connections && connection_repo.size() >= settings.max_connections)
				{
					SAS_LOG_TRACE(logger, "reuse already existing odbc connection");
					size_t min_conn_num = SIZE_MAX;
					for (auto & c : connection_repo)
					{
						if (c.second < min_conn_num)
						{
							conn = c.first;
							min_conn_num = c.second;
						}
					}
					if (!conn)
						conn = connection_repo.begin()->first;
					++connection_repo[conn];
				}
			}

			if (conn)
			{
				SAS_LOG_TRACE(logger, "test ODBC connection");

				SQLRETURN rc;
				SQLINTEGER tmp;
				if (SQL_SUCCEEDED(rc = SQLGetConnectAttr(conn->conn, SQL_ATTR_CONNECTION_DEAD, &tmp, 0, NULL)) && !tmp)
					return conn;

				SAS_LOG_INFO(logger, "ODBC connection has already gone. try to acquire a new one");
				delete conn;
				connection_registry.erase(Thread::getThreadId());
			}

			SAS_LOG_INFO(logger, "create new ODBC connection (#"+std::to_string(connection_repo.size()+1)+")");

			SQLHDBC _conn;
			if (!createODBCConnection(env, _conn, ec))
				return nullptr;

			connection_repo[conn = new Connection(env, _conn)] = 1;
			conn->connected = true;

			connection_registry[Thread::getThreadId()] = conn;

			return conn;
		}

		Connection * connection()
		{
			SAS_LOG_NDC();

			std::unique_lock<std::mutex> __locker(connection_repo_mut);
			auto * conn = connection_registry[Thread::getThreadId()];
			if (conn && conn->connected)
			{
				//ping?
				return conn;
			}

			connection_registry.erase(Thread::getThreadId());
			return nullptr;
		}

		void detach()
		{
			SAS_LOG_NDC();

			SAS_LOG_TRACE(logger, "detach odbc connection");

			std::unique_lock<std::mutex> __locker(connection_repo_mut);
			auto th_id = Thread::getThreadId();
			if (connection_registry.count(th_id))
			{
				auto conn = connection_registry[th_id];
				connection_registry.erase(th_id);
				if (connection_repo.count(conn))
				{
					auto & c = connection_repo[conn];
					if (c < 2)
					{
						connection_repo.erase(conn);
						delete conn;
					}
					else
						--c;
				}
			}
		}

	private:
		bool createODBCConnection(SQLHENV env, SQLHDBC & conn, ErrorCollector & ec)
		{
			SAS_LOG_NDC();

			SQLRETURN rc;

			SAS_LOG_TRACE(logger, "SQLAllocConnect");

			if (!SQL_SUCCEEDED(rc = SQLAllocConnect(env, &conn)))
			{
				auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "could not create connection handle: " + ODBCTools::getError(env, SQL_NULL_HANDLE, SQL_NULL_HANDLE, rc, ec));
				SAS_LOG_ERROR(logger, err);
				return false;
			}

			bool has_error = false;
			for (auto & o : this->connection_data.options)
			{
				if (o.first == "ACCESS_MODE")
				{
					SQLUINTEGER v;
					if (o.second == "READ_ONLY")
						v = SQL_MODE_READ_ONLY;
					else if (o.second == "READ_WRITE")
						v = SQL_MODE_READ_WRITE;
					else
					{
						auto err = ec.add(SAS_SQL__ERROR__INVALID_CONFIG_VALUE, "invalid config value for 'ACCESS_MODE': '" + o.second + "' ('READ_ONLY'/'READ_WRITE')");
						SAS_LOG_ERROR(logger, err);
						has_error = true;
						continue;
					}
                    rc= SQLSetConnectAttr(conn, SQL_ATTR_ACCESS_MODE, reinterpret_cast<SQLPOINTER>(v), 0);
				}
				else if (o.first == "CONNECTION_TIMEOUT")
				{
					auto v = (SQLUINTEGER)std::stoul(o.second);
                    rc = SQLSetConnectAttr(conn, SQL_ATTR_CONNECTION_TIMEOUT, reinterpret_cast<SQLPOINTER>(v), 0);
				}
				else if (o.first == "LOGIN_TIMEOUT")
				{
					auto v = (SQLUINTEGER)std::stoul(o.second);
                    rc = SQLSetConnectAttr(conn, SQL_ATTR_LOGIN_TIMEOUT, reinterpret_cast<SQLPOINTER>(v), 0);
				}
				else if (o.first == "CURRENT_CATALOG")
				{
					//auto v = (SQLUINTEGER)std::stoul(o.second);
					rc = SQLSetConnectAttr(conn, SQL_ATTR_CURRENT_CATALOG, (SQLPOINTER)o.second.c_str(), o.second.size());
				}
				else if (o.first == "METADATA_ID")
				{
					SQLUINTEGER v;
					if (o.second == "TRUE")
						v = SQL_TRUE;
					else if (o.second == "FALSE")
						v = SQL_FALSE;
					else
					{
						auto err = ec.add(SAS_SQL__ERROR__INVALID_CONFIG_VALUE, "invalid config value for 'METADATA_ID': '" + o.second + "' ('TRUE'/'FALSE')");
						SAS_LOG_ERROR(logger, err);
						has_error = true;
						continue;
					}
                    rc = SQLSetConnectAttr(conn, SQL_ATTR_METADATA_ID, reinterpret_cast<SQLPOINTER>(v), 0);
				}
				else if (o.first == "PACKET_SIZE")
				{
					//auto v = (SQLUINTEGER)std::stoul(o.second);
					rc = SQLSetConnectAttr(conn, SQL_ATTR_PACKET_SIZE, (SQLPOINTER)o.second.c_str(), o.second.size());
				}
				else if (o.first == "TRACEFILE")
				{
					//auto v = (SQLUINTEGER)std::stoul(o.second);
					rc = SQLSetConnectAttr(conn, SQL_ATTR_TRACEFILE, (SQLPOINTER)o.second.c_str(), o.second.size());
				}
				else if (o.first == "TRACE")
				{
					SQLUINTEGER v;
					if (o.second == "OFF")
						v = SQL_OPT_TRACE_OFF;
					else if (o.second == "ON")
						v = SQL_OPT_TRACE_ON;
					else
					{
						auto err = ec.add(SAS_SQL__ERROR__INVALID_CONFIG_VALUE, "invalid config value for 'TRACE': '" + o.second + "' ('OFF'/'ON')");
						SAS_LOG_ERROR(logger, err);
						has_error = true;
						continue;
					}
                    rc = SQLSetConnectAttr(conn, SQL_ATTR_TRACE, reinterpret_cast<SQLPOINTER>(v), 0);
				}
				else if (o.first == "CONNECTION_POOLING")
				{
					SQLUINTEGER v;
					if (o.second == "OFF")
						v = SQL_CP_OFF;
					else if (o.second == "ONE_PER_DRIVER")
						v = SQL_CP_ONE_PER_DRIVER;
					else if (o.second == "ONE_PER_HENV")
						v = SQL_CP_ONE_PER_HENV;
					else if (o.second == "DEFAULT")
						v = SQL_CP_DEFAULT;
					else
					{
						auto err = ec.add(SAS_SQL__ERROR__INVALID_CONFIG_VALUE, "invalid config value for 'CONNECTION_POOLING': '" + o.second + "' ('OFF'/'ONE_PER_DRIVER'/'ONE_PER_HENV'/'DEFAULT')");
						SAS_LOG_ERROR(logger, err);
						has_error = true;
						continue;
					}
                    rc = SQLSetConnectAttr(conn, SQL_ATTR_CONNECTION_POOLING, reinterpret_cast<SQLPOINTER>(v), 0);
				}
				else if (o.first == "CP_MATCH")
				{
					SQLUINTEGER v;
					if (o.second == "STRICT_MATCH")
						v = SQL_CP_STRICT_MATCH;
					else if (o.second == "RELAXED_MATCH")
						v = SQL_CP_RELAXED_MATCH;
					else if (o.second == "MATCH_DEFAULT")
						v = SQL_CP_MATCH_DEFAULT;
					else
					{
						auto err = ec.add(SAS_SQL__ERROR__INVALID_CONFIG_VALUE, "invalid config value for 'CP_MATCH': '" + o.second + "' ('STRICT_MATCH'/'RELAXED_MATCH'/'MATCH_DEFAULT')");
						SAS_LOG_ERROR(logger, err);
						has_error = true;
						continue;
					}
                    rc = SQLSetConnectAttr(conn, SQL_ATTR_CP_MATCH, reinterpret_cast<SQLPOINTER>(v), 0);
				}
				else
				{
					auto err = ec.add(SAS_SQL__ERROR__INVALID_CONFIG_VALUE, "invalid option: '" + o.first + "'");
					SAS_LOG_ERROR(logger, err);
					has_error = true;
					continue;
				}

				if (!SQL_SUCCEEDED(rc))
				{
					auto err = ec.add(SAS_SQL__ERROR__NOT_SUPPORTED, "could not set parameter '" + o.first + "': " + ODBCTools::getError(env, conn, SQL_NULL_HANDLE, rc, ec));
					SAS_LOG_ERROR(logger, err);
					has_error = true;
					continue;
				}
			}

			if (has_error)
			{
				SQLFreeConnect(conn);
				conn = NULL;
				return false;
			}

			if (!SQL_SUCCEEDED(rc = SQLConnect(conn,
				(SQLCHAR*)connection_data.dsn.c_str(), static_cast<SQLSMALLINT>(connection_data.dsn.length()),
				(SQLCHAR*)connection_data.user.c_str(), static_cast<SQLSMALLINT>(connection_data.user.length()),
				(SQLCHAR*)connection_data.passwd.c_str(), static_cast<SQLSMALLINT>(connection_data.passwd.length()))))
			{
				auto err = ec.add(SAS_SQL__ERROR__NOT_SUPPORTED, "could not connect: " + ODBCTools::getError(env, conn, SQL_NULL_HANDLE, rc, ec));
				SAS_LOG_ERROR(logger, err);
				return false;
			}

			return true;
		}

	} connectionManager;

};


ODBCConnector::ODBCConnector(SQLHENV env, const std::string & name, Application * app) : SQLConnector(), priv(new Priv(env, app, name))
{ }

ODBCConnector::~ODBCConnector()
{
	delete priv;
}

std::string ODBCConnector::name() const
{ return priv->name; }

const char * ODBCConnector::getServerType() const
{
	return priv->settings.info.db_type.length() ? priv->settings.info.db_type.c_str() : "(odbc)";
}

bool ODBCConnector::getServerInfo(std::string & generation, std::string & version, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	auto conn = priv->connectionManager.connection(ec);
	if (!conn)
		return false;

	std::unique_lock<std::mutex> __locker(conn->mut);

	SQLRETURN rc;

	char tmp[32];
	SQLSMALLINT len;
	if (!SQL_SUCCEEDED(rc = SQLGetInfo(conn->conn, SQL_DBMS_NAME, tmp, sizeof(tmp), &len)))
	{
		auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "could not DB name: " + ODBCTools::getError(priv->env, conn, SQL_NULL_HANDLE, rc, ec));
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}

	version = tmp;

	if (!SQL_SUCCEEDED(rc = SQLGetInfo(conn->conn, SQL_DRIVER_VER, tmp, sizeof(tmp), &len)))
	{
		auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "could not get driver version: " + ODBCTools::getError(priv->env, conn, SQL_NULL_HANDLE, rc, ec));
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}
	if (strlen(tmp) < 1)
	{
		version = generation = "(unknown)";
	}
	else
	{
		version = tmp;
		auto strl = str_split(version, '.');
		if (strl.size() < 2)
		{
			auto strl = str_split(version, '_');
			if (strl.size() < 2)
			{
				generation = "(unknown)";
			}
			else
				generation = strl.front();
		}
		else
			generation = strl.front();
	}

	return true;
}

bool ODBCConnector::hasFeature(Feature f, std::string & explanation)
{
	switch (f)
	{
	case SQLConnector::Feature::GetServerInfo:
	case SQLConnector::Feature::MultiThreading:
	case SQLConnector::Feature::Statement:
	case SQLConnector::Feature::BindingByPos:
		return true;
	case SQLConnector::Feature::GetNumRowsAffected:
		explanation = "functionality is only usable when execute DML statement";
		return false;
	case SQLConnector::Feature::Transaction:
		if (!priv->settings.info.transaction_support)
		{
			explanation = "transaction handling is not supported by DB '" + priv->settings.info.db_type + "'";
			return false;
		}
		return true;
	case SQLConnector::Feature::BindingByName:
		explanation = "not supported by ODBC";
		return false;
	case SQLConnector::Feature::SimpleQuery:
		explanation = "this feature is not natively supported by ODBC library, but implemented as using SQLStatement.";
		return true;
	case SQLConnector::Feature::GetLastGeneratedId:
		if (priv->settings.statementInjections.find(ODBC_Settings::StatementInjection::GetLastGeneratedId) != priv->settings.statementInjections.end())
		{
			explanation = "supported by statement injection";
			return true;
		}
		explanation = "not supported by ODBC, use DB specific solution instead";
		return false;
	case SQLConnector::Feature::GetSysDate:
		if (priv->settings.statementInjections.find(ODBC_Settings::StatementInjection::GetSysdate) != priv->settings.statementInjections.end())
		{
			explanation = "supported by statement injection";
			return true;
		}
		explanation = "not supported by ODBC, use DB specific solution instead";
		return false;
	default:
		explanation = "unknown feature: '" + std::to_string((int)f) + "'.";
	}

	return false;
}

bool ODBCConnector::init(const std::string & configPath, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	std::unique_lock<std::mutex> __locker(priv->connection_data_mut);
	auto cfg = priv->app->configReader();

	bool has_error(false);

	SAS_LOG_TRACE(priv->logger, "read connection data from config");

	long long ll_tmp;
	if (!cfg->getNumberEntry(configPath + "/MAX_CONNECTIONS", ll_tmp, 5, ec))
		has_error = true;
	else
		priv->settings.max_connections = (size_t)ll_tmp;
	if(!cfg->getStringEntry(configPath + "/USER", priv->connection_data.user, ec))
		has_error = true;
	if(!cfg->getStringEntry(configPath + "/PASSWD", priv->connection_data.passwd, ec))
		has_error = true;
	if (!cfg->getStringEntry(configPath + "/DSN", priv->connection_data.dsn, ec))
		has_error = true;
	if (!cfg->getNumberEntry(configPath + "/MAX_BUFFER_SIZE", ll_tmp, 33554432, ec)) //32M
		has_error = true;
	else
	{
		priv->settings.max_buffer_size = (size_t)ll_tmp;
		long long max = 2147483648; //2G
		if(ll_tmp < 0)
		{
			auto err = ec.add(SAS_SQL__ERROR__INVALID_CONFIG_VALUE, "MAX_BUFFER_SIZE cannot be less then 0");
			SAS_LOG_ERROR(priv->logger, err);
			has_error = true;
		}
		else if(ll_tmp > max)
		{
			auto err = ec.add(SAS_SQL__ERROR__INVALID_CONFIG_VALUE, "MAX_BUFFER_SIZE cannot be greater then " + std::to_string(max));
			SAS_LOG_ERROR(priv->logger, err);
			has_error = true;
		}
	}

	std::string tmp;
	if (cfg->getEntryAsString(configPath + "/ACCESS_MODE", tmp, ec))
		priv->connection_data.options.push_back(std::make_pair("ACCESS_MODE", tmp));
	if (cfg->getEntryAsString(configPath + "/CONNECTION_TIMEOUT", tmp, ec))
		priv->connection_data.options.push_back(std::make_pair("CONNECTION_TIMEOUT", tmp));
	if (cfg->getEntryAsString(configPath + "/LOGIN_TIMEOUT", tmp, ec))
		priv->connection_data.options.push_back(std::make_pair("LOGIN_TIMEOUT", tmp));
	if (cfg->getEntryAsString(configPath + "/CURRENT_CATALOG", tmp, ec))
		priv->connection_data.options.push_back(std::make_pair("CURRENT_CATALOG", tmp));
	if (cfg->getEntryAsString(configPath + "/METADATA_ID", tmp, ec))
		priv->connection_data.options.push_back(std::make_pair("METADATA_ID", tmp));
	if (cfg->getEntryAsString(configPath + "/PACKET_SIZE", tmp, ec))
		priv->connection_data.options.push_back(std::make_pair("PACKET_SIZE", tmp));
	if (cfg->getEntryAsString(configPath + "/TRACEFILE", tmp, ec))
		priv->connection_data.options.push_back(std::make_pair("TRACEFILE", tmp));
	if (cfg->getEntryAsString(configPath + "/TRACE", tmp, ec))
		priv->connection_data.options.push_back(std::make_pair("TRACE", tmp));
	if (cfg->getEntryAsString(configPath + "/CONNECTION_POOLING", tmp, ec))
		priv->connection_data.options.push_back(std::make_pair("CONNECTION_POOLING", tmp));
	if (cfg->getEntryAsString(configPath + "/CP_MATCH", tmp, ec))
		priv->connection_data.options.push_back(std::make_pair("CP_MATCH", tmp));

	{
		std::string str;
		if(!cfg->getStringEntry(configPath + "/INT64_BIND_RULE", str, "normal", ec))
			has_error = true;
		else
		{
			if (str == "not_supported")
				priv->settings.int64BindRule = ODBC_Settings::Int64BindRule::NotSupported;
			else if (str == "normal")
				priv->settings.int64BindRule = ODBC_Settings::Int64BindRule::Normal;
			else if (str == "as_int32")
				priv->settings.int64BindRule = ODBC_Settings::Int64BindRule::AsInt32;
			else if (str == "as_string")
				priv->settings.int64BindRule = ODBC_Settings::Int64BindRule::AsString;
			else if (str == "as_int32_or_as_string")
				priv->settings.int64BindRule = ODBC_Settings::Int64BindRule::AsInt32_or_AsString;
			else
			{
				auto err = ec.add(SAS_SQL__ERROR__INVALID_CONFIG_VALUE, "invalid config value: " + str);
				SAS_LOG_ERROR(priv->logger, err);
				has_error = true;
			}
		}
	}

	if (cfg->getEntryAsString(configPath + "/STATEMENT/GET_LAST_GENERATED_ID", tmp, ec))
		priv->settings.statementInjections[ODBC_Settings::StatementInjection::GetLastGeneratedId] = tmp;
	if (cfg->getEntryAsString(configPath + "/STATEMENT/GET_SYSDATE", tmp, ec))
		priv->settings.statementInjections[ODBC_Settings::StatementInjection::GetSysdate] = tmp;

	std::vector<std::string> mascros;
	if (cfg->getEntryAsStringList(configPath + "/MACROS", mascros, ec))
		for (auto& name : mascros)
		{
			std::vector<std::string> args;
			cfg->getEntryAsStringList(configPath + "/MACRO/" + name + "/ARGS", args, ec);

			if (cfg->getEntryAsString(configPath + "/MACRO/" + name + "/TMPL", tmp, ec))
				priv->settings.macros[name] = std::make_tuple(tmp, args);
		}

	if(has_error)
	{
		SAS_LOG_DEBUG(priv->logger, "connection data are not complete");
		return false;
	}


	if (!priv->connectionManager.init(priv->env, ec))
		return false;

	return true;
}

bool ODBCConnector::connect(ErrorCollector & ec)
{
	SAS_LOG_NDC();

	Priv::ConnectionManager::Connection * conn;
	if (!(conn = priv->connectionManager.connection(ec)))
		return false;

	{
		SQLRETURN rc;
		std::unique_lock<std::mutex> __locker(conn->mut);

		SQLHANDLE l_stmt = NULL;
		if (!SQL_SUCCEEDED(rc = SQLAllocHandle(SQL_HANDLE_STMT, conn->conn, &l_stmt)))
		{
			auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "could not allocate statement: " + ODBCTools::getError(priv->env, conn->conn, SQL_NULL_HANDLE, rc, ec));
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}
		{
			SQLSMALLINT cols_size;
			SQLSMALLINT dt_sub;
			if (!SQL_SUCCEEDED(rc = SQLGetTypeInfo(l_stmt, SQL_TIMESTAMP)) ||
				!SQL_SUCCEEDED(rc = SQLFetch(l_stmt)) ||
				!SQL_SUCCEEDED(rc = SQLGetData(l_stmt, 3 /*COLUMN_SIZE*/, SQL_C_SHORT, &cols_size, sizeof(cols_size), 0)) ||
				!SQL_SUCCEEDED(rc = SQLGetData(l_stmt, 17 /*SQL_DATETIME_SUB*/, SQL_C_SHORT, &dt_sub, sizeof(dt_sub), 0)))
			{
				auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "could not get type information: " + ODBCTools::getError(priv->env, conn->conn, l_stmt, rc, ec));
				SAS_LOG_ERROR(priv->logger, err);
				SQLFreeHandle(SQL_HANDLE_STMT, l_stmt);
				return false;
			}
			priv->settings.info.dtprec = 3 * (int)dt_sub; // FIXME: ???
//			priv->settings.info.dtprec =   cols_size;
		}
		SQLFreeHandle(SQL_HANDLE_STMT, l_stmt);

		{
			SQLCHAR tmp[32];
			SQLSMALLINT len;
			if (!SQL_SUCCEEDED(rc = SQLGetInfo(conn->conn, SQL_DBMS_NAME, tmp, sizeof(tmp), &len)))
			{
				auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "could not get DB name: " + ODBCTools::getError(priv->env, conn, SQL_NULL_HANDLE, rc, ec));
				SAS_LOG_ERROR(priv->logger, err);
				return false;
			}
			priv->settings.info.db_type = std::string((const char *)tmp);
		}

		{
			SQLUSMALLINT tmp;
			SQLSMALLINT len;
			if (!SQL_SUCCEEDED(rc = SQLGetInfo(conn->conn, SQL_TXN_CAPABLE, &tmp, sizeof(tmp), &len)))
			{
				auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "could not get info about transaction support: " + ODBCTools::getError(priv->env, conn, SQL_NULL_HANDLE, rc, ec));
				SAS_LOG_ERROR(priv->logger, err);
				return false;
			}
			priv->settings.info.transaction_support = tmp != SQL_TC_NONE;
		}
	}

	priv->connectionManager.detach();
	return true;
}

SQLStatement * ODBCConnector::createStatement(ErrorCollector & ec)
{
	SAS_LOG_NDC();
	auto conn = priv->connectionManager.connection(ec);
	if (!conn)
		return nullptr;

	std::unique_lock<std::mutex> __locker(conn->mut);

	auto st = new ODBCStatement(this);
	if (!st->init(ec))
	{
		delete st;
		return nullptr;
	}
	return st;
}

bool ODBCConnector::exec(const std::string & statement, SQLResult *& res, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	auto conn = priv->connectionManager.connection(ec);
	if (!conn)
		return false;

	std::unique_lock<std::mutex> __locker(conn->mut);

	SAS_LOG_VAR(priv->logger, statement);

	auto stmt = std::make_shared<ODBCStatement>(this);
	if (!stmt->init(ec) ||
	    !stmt->prepare(statement, ec) ||
	    !stmt->exec(ec))
		return false;

	res = new SQLStatementResult(stmt);

	return true;
}

bool ODBCConnector::exec(const std::string & statement, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	auto conn = priv->connectionManager.connection(ec);
	if (!conn)
		return false;

	std::unique_lock<std::mutex> __locker(conn->mut);

	SAS_LOG_VAR(priv->logger, statement);

	auto stmt = std::make_shared<ODBCStatement>(this);
	if (!stmt->prepare(statement, ec) ||
		!stmt->execDML(ec))
		return false;

	return true;
}

void ODBCConnector::detach()
{
	priv->connectionManager.detach();
}

Logging::LoggerPtr ODBCConnector::logger() const
{
	return priv->logger;
}

const ODBC_Settings & ODBCConnector::settings() const
{
	return priv->settings;
}

std::mutex & ODBCConnector::mutex()
{
	auto conn = priv->connectionManager.connection();
	if (!conn)
	{
		SAS_LOG_NDC();
		SAS_LOG_ASSERT(priv->logger, conn, "connection is not yet activated");
	}
	return conn->mut;
}

bool ODBCConnector::activate(ErrorCollector & ec)
{
	SAS_LOG_NDC();
	return priv->connectionManager.connection(ec) != nullptr;
}

void ODBCConnector::lock()
{
	auto conn = priv->connectionManager.connection();
	if (!conn)
	{
		SAS_LOG_NDC();
		SAS_LOG_ASSERT(priv->logger, conn, "connection is not yet activated");
	}
	conn->external_mut.lock();
}

void ODBCConnector::unlock()
{
	auto conn = priv->connectionManager.connection();
	if (!conn)
	{
		SAS_LOG_NDC();
		SAS_LOG_ASSERT(priv->logger, conn, "connection is not yet activated");
	}
	conn->external_mut.unlock();
}

bool ODBCConnector::startTransaction(ErrorCollector & ec)
{
	SAS_LOG_NDC();
	auto conn = priv->connectionManager.connection();
	if (!conn)
	{
		auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "connection is not specified for the thread");
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}

	SQLRETURN rc;

	SAS_LOG_TRACE(priv->logger, "SQLSetConnectAttr");
	if (!SQL_SUCCEEDED(rc = SQLSetConnectAttr(conn->conn, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)FALSE, 0)))
	{
		auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "could start transaction: " + ODBCTools::getError(priv->env, conn->conn, SQL_NULL_HANDLE, rc, ec));
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}

	return true;
}

bool ODBCConnector::commit(ErrorCollector & ec)
{
	SAS_LOG_NDC();
	auto conn = priv->connectionManager.connection();
	if(!conn)
	{
		auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "connection is not specified for the thread");
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}

	SQLRETURN rc;

	SAS_LOG_TRACE(priv->logger, "SQLEndTran");
	if (!SQL_SUCCEEDED(rc = SQLEndTran(SQL_HANDLE_DBC, conn->conn, SQL_COMMIT)))
	{
		auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "could not commit transaction: " + ODBCTools::getError(priv->env, conn->conn, SQL_NULL_HANDLE, rc, ec));
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}

	return true;
}

bool ODBCConnector::rollback(ErrorCollector & ec)
{
	SAS_LOG_NDC();
	auto conn = priv->connectionManager.connection();
	if(!conn)
	{
		auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "connection is not specified for the thread");
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}
	SQLRETURN rc;

	SAS_LOG_TRACE(priv->logger, "SQLEndTran");
	if (!SQL_SUCCEEDED(rc = SQLEndTran(SQL_HANDLE_DBC, conn->conn, SQL_ROLLBACK)))
	{
		auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "could not rollback transaction: " + ODBCTools::getError(priv->env, conn->conn, SQL_NULL_HANDLE, rc, ec));
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}

	return true;
}

SQLHDBC ODBCConnector::conn(ErrorCollector & ec)
{
	auto c = priv->connectionManager.connection(ec);
	return c ? c->conn : nullptr;
}

SQLHDBC ODBCConnector::conn()
{
	auto c = priv->connectionManager.connection();
	return c ? c->conn : nullptr;
}

SQLHENV ODBCConnector::env() const
{
	return priv->env;
}

std::string ODBCConnector::getErrorText(SQLHSTMT stmt, SQLRETURN rc, ErrorCollector & ec)
{
	return ODBCTools::getError(priv->env, conn(), stmt, rc, ec);
}

std::string ODBCConnector::getErrorText()
{
	NullEC ec;
	return getErrorText(SQL_NULL_HANDLE, -1, ec);
}

bool ODBCConnector::appendCompletionValue(const std::string& command, const std::vector<std::string>& args, std::string& ret, ErrorCollector& ec) const //final override
{
	auto it = priv->settings.macros.find(command);
	if (it == priv->settings.macros.end())
	{
		auto err = ec.add(-1, "unsupported macro: '" + command + "'");
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}

	auto tmpl = std::get<0>(it->second);
	size_t i = 0;
	for(auto & a : std::get<1>(it->second))
		tmpl = std::regex_replace(tmpl, std::regex("\\$\\(" + a + "\\)|\\$\\{" + a + "\\}"), i < args.size() ? args[i++] : std::string());

	ret += tmpl;

	return true;
}

}
