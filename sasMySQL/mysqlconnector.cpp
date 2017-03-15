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

#include "mysqlresult.h"
#include "mysqlstatement.h"
#include <sasCore/thread.h>

#include <vector>
#include <list>
#include <mutex>
#include <assert.h>

namespace SAS {

struct MySQLConnector_priv
{
	MySQLConnector_priv()
	{ }

	std::string name;
	struct ConnectionData
	{
		ConnectionData() : port(0) //, connected(false)
		{ }

		std::string host;
		std::string user;
		std::string passwd;
		std::string db;
		unsigned int port; // optional
		std::string unix_socket; // optional

//		bool connected;
	} connection_data;
	std::mutex connection_data_mut;

	Logging::LoggerPtr logger;

	Application * app;

	struct Connection
	{
		Connection() : my(mysql_init(nullptr)), connected(false)
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
		bool connected;
	} _conn;


	std::mutex connection_repo_mut;
	std::map<ThreadId, Connection*> connection_registry;
	std::map<Connection*, size_t> connection_repo;

	Connection * conn(ErrorCollector & ec)
	{
		SAS_LOG_NDC();

		std::unique_lock<std::mutex> __locker(connection_repo_mut);
		auto & conn = connection_registry[Thread::getThreadId()];
		if (conn && conn->connected)
			return conn;

		if (!conn)
		{
			if (settings.max_connections && connection_repo.size() >= settings.max_connections)
			{
				SAS_LOG_TRACE(logger, "reuse already existing mysql connection");
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
			else
			{
				SAS_LOG_INFO(logger, "create new mysql connection (#"+std::to_string(connection_repo.size()+1)+")");
				connection_repo[conn = new Connection] = 1;
			}
			connection_registry[Thread::getThreadId()] = conn;
		}

		assert(conn);

		if (conn->connected)
			return conn;

		SAS_LOG_ASSERT(logger, conn->my, "mysql pointer must not be null"); // checked only here!

		SAS_LOG_VAR(logger, connection_data.host);
		SAS_LOG_VAR(logger, connection_data.user);
		SAS_LOG_VAR(logger, connection_data.passwd);
		SAS_LOG_VAR(logger, connection_data.db);
		SAS_LOG_VAR(logger, connection_data.port);
		SAS_LOG_VAR(logger, connection_data.unix_socket);

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
				auto err = ec.add(-1, std::string("could not connect to MySQL service: ") + mysql_error(conn->my));
				SAS_LOG_ERROR(logger, err);
				detach();
				return nullptr;
			}
			conn->connected = true;
			SAS_LOG_DEBUG(logger, "mysql connection has been successfully built");
		}

		return conn;
	}

	Connection * conn()
	{
		std::unique_lock<std::mutex> __locker(connection_repo_mut);
		auto * conn = connection_registry[Thread::getThreadId()];
		if (conn && conn->connected)
			return conn;

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

	MySQL_Settings settings;

};


MySQLConnector::MySQLConnector(const std::string & name, Application * app) : SQLConnector(), priv(new MySQLConnector_priv)
{
	priv->app = app;
	priv->logger = Logging::getLogger("SAS.MySQLConnector." + name);
	priv->name= name;
}

MySQLConnector::~MySQLConnector()
{
	delete priv;
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
			auto err = ec.add(-1, "MAX_BUFFER_SIZE cannot be less then 0");
			SAS_LOG_ERROR(priv->logger, err);
			has_error = true;
		}
		else if(ll_tmp > max)
		{
			auto err = ec.add(-1, "MAX_BUFFER_SIZE cannot be greater then " + std::to_string(max));
			SAS_LOG_ERROR(priv->logger, err);
			has_error = true;
		}
	}

	if(cfg->getNumberEntry(configPath + "/PORT", ll_tmp, 0, ec))
		priv->connection_data.port = (size_t)ll_tmp;
	cfg->getStringEntry(configPath + "/UNIX_SOCKET", priv->connection_data.unix_socket, priv->connection_data.unix_socket, ec);

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

	if(!priv->conn(ec))
		return false;
	
	priv->detach();
	return true;
}

SQLStatement * MySQLConnector::createStatement(ErrorCollector & ec)
{
	SAS_LOG_NDC();
	auto conn = priv->conn(ec);
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
	auto conn = priv->conn(ec);
	if (!conn)
		return false;

	std::unique_lock<std::mutex> __locker(conn->mut);

	SAS_LOG_VAR(priv->logger, statement);

	SAS_LOG_TRACE(priv->logger, "mysql_query");
	if(mysql_query(conn->my, statement.c_str()))
	{
		auto err = ec.add(-1, std::string("could execute SQL statement: ") + mysql_error(conn->my));
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
	auto conn = priv->conn(ec);
	if (!conn)
		return false;

	std::unique_lock<std::mutex> __locker(conn->mut);

	SAS_LOG_VAR(priv->logger, statement);

	SAS_LOG_TRACE(priv->logger, "mysql_query");
	if(mysql_query(conn->my, statement.c_str()))
	{
		auto err = ec.add(-1, std::string("could execute SQL statement: ") + mysql_error(conn->my));
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}

	return true;
}

void MySQLConnector::detach()
{
	priv->detach();
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
	auto conn = priv->conn();
	if (!conn)
	{
		SAS_LOG_NDC();
		SAS_LOG_ASSERT(priv->logger, conn, "connection is not yet activated");
	}
	return conn->mut;
}

void MySQLConnector::lock()
{
	auto conn = priv->conn();
	if (!conn)
	{
		SAS_LOG_NDC();
		SAS_LOG_ASSERT(priv->logger, conn, "connection is not yet activated");
	}
	conn->external_mut.lock();
}

void MySQLConnector::unlock()
{
	auto conn = priv->conn();
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
