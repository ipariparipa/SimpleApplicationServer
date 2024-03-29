/*
    This file is part of sasMySQL.

    sasMySQL is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasMySQL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasMySQL.  If not, see <http://www.gnu.org/licenses/>
 */

#include "mysqlconnector.h"

#include SAS_MYSQL__MYSQL_H

#include <sasCore/application.h>
#include <sasCore/configreader.h>
#include <sasSQL/errorcodes.h>
#include <sasSQL/sqlstatement.h>
#include <sasSQL/sqldatetime.h>

#include "mysqlresult.h"
#include "mysqlstatement.h"
#include <sasCore/thread.h>

#include <vector>
#include <list>
#include <mutex>
#include <memory>
#include <assert.h>

namespace SAS {

struct MySQLConnector::Priv
{
	Priv(Application * app_, const std::string & name_) :
		app(app_),
		name(name_),
		logger(Logging::getLogger("SAS.MySQLConnector." + name_)),
		connectionManager(settings, connection_data, name_)
	{ }

	Application * app;
	std::string name;
	Logging::LoggerPtr logger;
	MySQL_Settings settings;

	struct ConnectionData
	{
		std::string host;
		std::string user;
		std::string passwd;
		std::string db;
		unsigned int port = 0; // optional
		std::string unix_socket; // optional
        std::string charset;
    } connection_data;
	std::mutex connection_data_mut;

	struct ConnectionManager //: public TimerThread
	{
		const MySQL_Settings & settings;
		const ConnectionData & connection_data;
		Logging::LoggerPtr logger;

		ConnectionManager(const MySQL_Settings & settings_, const ConnectionData & connection_data_, const std::string & name) :
			settings(settings_),
			connection_data(connection_data_),
			logger(Logging::getLogger("SAS.MySQLConnector." + name + ".ConnectionManager."))
		{ }

		struct Connection
		{
			Connection() : my(mysql_init(nullptr))
			{ }

			~Connection()
			{
				std::unique_lock<std::mutex> __locker(mut);
				if (my)
					mysql_close(my);
			}

			MYSQL * my;
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

			Connection * conn = nullptr;
			{
				std::unique_lock<std::mutex> __locker(connection_repo_mut);
				auto & _conn = connection_registry[Thread::getThreadId()];
				if (!_conn)
				{
					if (settings.max_connections && connection_repo.size() >= settings.max_connections)
					{
						SAS_LOG_TRACE(logger, "reuse already existing mysql connection");
						size_t min_conn_num = SIZE_MAX;
						for (auto & c : connection_repo)
						{
							if (c.second < min_conn_num)
							{
								conn = _conn = c.first;
								min_conn_num = c.second;
							}
						}
						if (!_conn)
							conn = _conn = connection_repo.begin()->first;
						++connection_repo[conn];
					}
					else
					{
						SAS_LOG_INFO(logger, "create new mysql connection (#" + std::to_string(connection_repo.size() + 1) + ")");
						connection_repo[conn = new Connection] = 1;
					}
					connection_registry[Thread::getThreadId()] = conn;
				}
				else
					conn = _conn;
			}
			assert(conn);

			if (conn->connected)
			{
				SAS_LOG_TRACE(logger, "test MySQL connection");
				if(!mysql_ping(conn->my))
					return conn;
				conn->connected = false;
				SAS_LOG_INFO(logger, "MySQL connection has already gone. try to rebuild connection");
			}

			//not (yet) connected or connection has already gone

			SAS_LOG_ASSERT(logger, conn->my, "mysql pointer must not be null"); // checked only here!

			SAS_LOG_VAR(logger, connection_data.host);
			SAS_LOG_VAR(logger, connection_data.user);
			SAS_LOG_VAR(logger, connection_data.passwd);
			SAS_LOG_VAR(logger, connection_data.db);
			SAS_LOG_VAR(logger, connection_data.port);
			SAS_LOG_VAR(logger, connection_data.unix_socket);
            SAS_LOG_VAR(logger, connection_data.charset);

			{
				std::unique_lock<std::mutex> __locker(conn->mut);
				SAS_LOG_TRACE(logger, "mysql_real_connect");
				if (!mysql_real_connect(conn->my,
					connection_data.host.c_str(),
					connection_data.user.c_str(),
					connection_data.passwd.c_str(),
					connection_data.db.c_str(),
					connection_data.port,
					connection_data.unix_socket.size() ? connection_data.unix_socket.c_str() : NULL,
					0))
				{
					auto err = ec.add(SAS_SQL__ERROR__CANNOT_CONNECT_TO_DB_SEVICE, std::string("could not connect to MySQL service: ") + mysql_error(conn->my));
					SAS_LOG_ERROR(logger, err);
					__locker.unlock();
					detach();
					return nullptr;
				}

                if(connection_data.charset.length() && mysql_set_character_set(conn->my, connection_data.charset.c_str()) != 0)
                {
                    auto err = ec.add(SAS_SQL__ERROR__CANNOT_CONNECT_TO_DB_SEVICE, std::string("could not set character set: ") + mysql_error(conn->my));
                    SAS_LOG_ERROR(logger, err);
                    __locker.unlock();
                    detach();
                    return nullptr;
                }

				conn->connected = true;
				SAS_LOG_DEBUG(logger, "mysql connection has been successfully built");
			}

			return conn;
		}

		Connection * connection()
		{
			SAS_LOG_NDC();

			std::unique_lock<std::mutex> __locker(connection_repo_mut);
			auto * conn = connection_registry[Thread::getThreadId()];
			if (conn && conn->connected)
			{
				/*
				if(mysql_ping(conn->my))
				{
					SAS_LOG_INFO(logger, "MySQL connection has already gone.");
					conn->connected = false;
					connection_registry.erase(Thread::getThreadId());
					return nullptr;
				}
				*/
				return conn;
			}

			connection_registry.erase(Thread::getThreadId());
			return nullptr;
		}

		void detach()
		{
			SAS_LOG_NDC();

			SAS_LOG_TRACE(logger, "detach mysql connection");

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
	} connectionManager;

};


MySQLConnector::MySQLConnector(const std::string & name, Application * app) : SQLConnector(), priv(new Priv(app, name))
{ }

MySQLConnector::~MySQLConnector()
{
	delete priv;
}

bool MySQLConnector::getServerInfo(std::string & generation, std::string & version, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	auto conn = priv->connectionManager.connection(ec);
	if (!conn)
		return false;

	std::unique_lock<std::mutex> __locker(conn->mut);

	SAS_LOG_TRACE(priv->logger, "mysql_get_server_info");
	auto _ver = mysql_get_server_info(conn->my);
	SAS_LOG_TRACE(priv->logger, "mysql_get_server_version");
	auto _num_ver = mysql_get_server_version(conn->my);
	if (!_ver || ! _num_ver)
	{
		auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, std::string("get server info: ") + mysql_error(conn->my));
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}
	version = _ver;
	auto _major = _num_ver / 10000;
	_num_ver -= _major * 10000;
	auto _minor = _num_ver / 100;
	generation = std::to_string(_major) + "." + std::to_string(_minor);
	return true;
}

bool MySQLConnector::hasFeature(Feature f, std::string & explanation)
{
	switch(f)
	{
	case SQLConnector::Feature::GetServerInfo:
	case SQLConnector::Feature::MultiThreading:
	case SQLConnector::Feature::Transaction:
	case SQLConnector::Feature::GetSysDate:
	case SQLConnector::Feature::Statement:
	case SQLConnector::Feature::BindingByPos:
	case SQLConnector::Feature::GetNumRowsAffected:
		return true;
	case SQLConnector::Feature::SimpleQuery:
		explanation = "data types of results are not handled properly by MySQL connector. for complex SQL statement this is suggested to use SQLStatement instead.";
		return true;
	case SQLConnector::Feature::BindingByName:
		return false;
	case SQLConnector::Feature::GetLastGeneratedId:
		explanation = "the related schema, table and field cannot be specified.";
		return true;
	default:
		explanation = "unknown feature: '" + std::to_string((int)f) + "'.";
	}

	return false;
}

std::string MySQLConnector::name() const
{ return priv->name; }

bool MySQLConnector::init(const std::string & configPath, ErrorCollector & ec)
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
	if (!cfg->getStringEntry(configPath + "/HOST", priv->connection_data.host, ec))
		has_error = true;
	if(!cfg->getStringEntry(configPath + "/USER", priv->connection_data.user, ec))
		has_error = true;
	if(!cfg->getStringEntry(configPath + "/PASSWD", priv->connection_data.passwd, ec))
		has_error = true;
	if(!cfg->getStringEntry(configPath + "/DB", priv->connection_data.db, ec))
		has_error = true;
	if(!cfg->getNumberEntry(configPath + "/MAX_BUFFER_SIZE", ll_tmp, 33554432, ec)) //32M
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

	if(cfg->getNumberEntry(configPath + "/PORT", ll_tmp, 0, ec))
		priv->connection_data.port = (size_t)ll_tmp;

    cfg->getStringEntry(configPath + "/UNIX_SOCKET", priv->connection_data.unix_socket, priv->connection_data.unix_socket, ec);
    cfg->getStringEntry(configPath + "/CHARACTER_SET", priv->connection_data.charset, priv->connection_data.charset, ec);

	if(has_error)
	{
		SAS_LOG_DEBUG(priv->logger, "connection data are not complete");
		return false;
	}

	return true;
}

bool MySQLConnector::connect(ErrorCollector & ec)
{
	SAS_LOG_NDC();

	if(!priv->connectionManager.connection(ec))
		return false;
	
	priv->connectionManager.detach();
	return true;
}

SQLStatement * MySQLConnector::createStatement(ErrorCollector & ec)
{
	SAS_LOG_NDC();
	auto conn = priv->connectionManager.connection(ec);
	if (!conn)
		return nullptr;

	std::unique_lock<std::mutex> __locker(conn->mut);

	SAS_LOG_TRACE(priv->logger, "mysql_stmt_init");
	MYSQL_STMT * stmt = mysql_stmt_init(conn->my);
	return new MySQLStatement(stmt, this);
}

bool MySQLConnector::exec(const std::string & statement, SQLResult *& res, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	auto conn = priv->connectionManager.connection(ec);
	if (!conn)
		return false;

	std::unique_lock<std::mutex> __locker(conn->mut);

	SAS_LOG_VAR(priv->logger, statement);

	SAS_LOG_TRACE(priv->logger, "mysql_query");
	if(mysql_query(conn->my, statement.c_str()))
	{
		auto err = ec.add(SAS_SQL__ERROR__CANNOT_EXECUTE_STATEMENT, std::string("could execute SQL statement: ") + mysql_error(conn->my));
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}

	SAS_LOG_TRACE(priv->logger, "mysql_store_result");
	res = new MySQLResult(this, mysql_store_result(conn->my));

	return true;
}

bool MySQLConnector::exec(const std::string & statement, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	auto conn = priv->connectionManager.connection(ec);
	if (!conn)
		return false;

	std::unique_lock<std::mutex> __locker(conn->mut);

	SAS_LOG_VAR(priv->logger, statement);

	SAS_LOG_TRACE(priv->logger, "mysql_query");
	if(mysql_query(conn->my, statement.c_str()))
	{
		auto err = ec.add(SAS_SQL__ERROR__CANNOT_EXECUTE_STATEMENT, std::string("could execute SQL statement: ") + mysql_error(conn->my));
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}

	return true;
}

void MySQLConnector::detach()
{
	priv->connectionManager.detach();
}

Logging::LoggerPtr MySQLConnector::logger() const
{
	return priv->logger;
}

const MySQL_Settings & MySQLConnector::settings() const
{
	return priv->settings;
}

std::mutex & MySQLConnector::mutex()
{
	auto conn = priv->connectionManager.connection();
	if (!conn)
	{
		SAS_LOG_NDC();
		SAS_LOG_ASSERT(priv->logger, conn, "connection is not yet activated");
	}
	return conn->mut;
}

bool MySQLConnector::activate(ErrorCollector & ec)
{
	SAS_LOG_NDC();
	return priv->connectionManager.connection(ec) != nullptr;
}

void MySQLConnector::lock()
{
	auto conn = priv->connectionManager.connection();
	if (!conn)
	{
		SAS_LOG_NDC();
		SAS_LOG_ASSERT(priv->logger, conn, "connection is not yet activated");
	}
	conn->external_mut.lock();
}

void MySQLConnector::unlock()
{
	auto conn = priv->connectionManager.connection();
	if (!conn)
	{
		SAS_LOG_NDC();
		SAS_LOG_ASSERT(priv->logger, conn, "connection is not yet activated");
	}
	conn->external_mut.unlock();
}

bool MySQLConnector::startTransaction(ErrorCollector & ec)
{
	SAS_LOG_NDC();
	return exec("start transaction", ec);
}

bool MySQLConnector::commit(ErrorCollector & ec)
{
	SAS_LOG_NDC();
	return exec("commit", ec);
}

bool MySQLConnector::rollback(ErrorCollector & ec)
{
	SAS_LOG_NDC();
	return exec("rollback", ec);
}

}
