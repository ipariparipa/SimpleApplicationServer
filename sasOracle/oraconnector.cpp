/*
    This file is part of sasOracle.

    sasOracle is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasOracle is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasOracle.  If not, see <http://www.gnu.org/licenses/>
 */

#include "oraconnector.h"

#include SAS_ORACLE__DPI_H

#include <sasCore/application.h>
#include <sasCore/configreader.h>
#include <sasSQL/errorcodes.h>
#include <sasSQL/sqlstatement.h>
#include <sasSQL/sqldatetime.h>

#include "oraresult.h"
#include "orastatement.h"
#include "oratools.h"
#include <sasCore/thread.h>

#include <vector>
#include <list>
#include <mutex>
#include <memory>
#include <assert.h>
#include <string.h>

namespace SAS {

struct OraConnector::Priv
{
	Priv(OraConnector * that, dpiContext * ctx_, Application * app_, const std::string & name_) :
		ctx(ctx_),
		app(app_),
		name(name_),
		logger(Logging::getLogger("SAS.OraConnector." + name_)),
		connectionManager(settings, connection_data, name_)
	{ }

	dpiContext * ctx;
	Application * app;
	std::string name;
	Logging::LoggerPtr logger;
	Oracle_Settings settings;

	struct ConnectionData
	{
		std::string user;
		std::string passwd;
		std::string connstr;
	} connection_data;
	std::mutex connection_data_mut;

	struct ConnectionManager //: public TimerThread
	{
		const Oracle_Settings & settings;
		const ConnectionData & connection_data;
		Logging::LoggerPtr logger;
		dpiContext * ctx = NULL;
		dpiPool * pool = NULL;

		ConnectionManager(const Oracle_Settings & settings_, const ConnectionData & connection_data_, const std::string & name) :
			settings(settings_),
			connection_data(connection_data_),
			logger(Logging::getLogger("SAS.OraConnector." + name + ".ConnectionManager."))
		{ }

		bool init(dpiContext * ctx_, ErrorCollector & ec)
		{
			SAS_LOG_NDC();
			ctx = ctx_;
			dpiCommonCreateParams params;
			memset(&params, 0, sizeof(dpiCommonCreateParams));
			SAS_LOG_TRACE(logger, "dpiPool_create");
			if (dpiPool_create(ctx,
				connection_data.user.c_str(), connection_data.user.length(),
				connection_data.passwd.c_str(), connection_data.passwd.length(),
				connection_data.connstr.c_str(), connection_data.connstr.length(),
				&params, NULL, &pool) != DPI_SUCCESS)
			{
				auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "could not create connection pool: " + OraTools::toString(ctx));
				SAS_LOG_ERROR(logger, err);
				return false;
			}

			return true;
		}

		struct Connection
		{
			Connection(dpiContext * ctx_, dpiConn * conn_) : ctx(ctx_), conn(conn_)
			{ }

			~Connection()
			{
				std::unique_lock<std::mutex> __locker(mut);
				dpiConn_release(conn);
			}

			dpiContext * ctx;
			dpiConn * conn;
			std::mutex mut;
			std::mutex external_mut;
			size_t connected = 0;
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
					SAS_LOG_TRACE(logger, "reuse already existing oracle connection");
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
				SAS_LOG_TRACE(logger, "test Oracle connection");
				if (dpiConn_ping(conn->conn) == DPI_SUCCESS)
					return conn;

				SAS_LOG_INFO(logger, "oracle connection has already gone. try to acquire a new one");
				delete conn;
				connection_registry.erase(Thread::getThreadId());
			}
			else
			{
				SAS_LOG_INFO(logger, "create new oracle connection (#"+std::to_string(connection_repo.size()+1)+")");
				assert(pool);
				dpiConn * _conn;
				SAS_LOG_TRACE(logger, "dpiPool_acquireConnection");
				if (dpiPool_acquireConnection(pool, NULL, 0, NULL, 0, NULL, &_conn) != DPI_SUCCESS)
				{
					auto err = ec.add(SAS_SQL__ERROR__CANNOT_CONNECT_TO_DB_SEVICE, "could not acquire connection from pool: " + OraTools::toString(ctx));
					SAS_LOG_ERROR(logger, err);
					return nullptr;
				}

				connection_repo[conn = new Connection(ctx, _conn)] = 1;
			}
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

			SAS_LOG_TRACE(logger, "detach oracle connection");

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


OraConnector::OraConnector(dpiContext * ctx, const std::string & name, Application * app) : SQLConnector(), priv(new Priv(this, ctx, app, name))
{ }

OraConnector::~OraConnector()
{
	delete priv;
}

std::string OraConnector::name() const
{ return priv->name; }

bool OraConnector::init(const std::string & configPath, ErrorCollector & ec)
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
	if(!cfg->getStringEntry(configPath + "/CONNSTR", priv->connection_data.connstr, ec))
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

	if(has_error)
	{
		SAS_LOG_DEBUG(priv->logger, "connection data are not complete");
		return false;
	}

	return priv->connectionManager.init(priv->ctx, ec);
}

bool OraConnector::connect(ErrorCollector & ec)
{
	SAS_LOG_NDC();

	if(!priv->connectionManager.connection(ec))
		return false;
	
	priv->connectionManager.detach();
	return true;
}

SQLStatement * OraConnector::createStatement(ErrorCollector & ec)
{
	SAS_LOG_NDC();
	auto conn = priv->connectionManager.connection(ec);
	if (!conn)
		return nullptr;

	std::unique_lock<std::mutex> __locker(conn->mut);

	return new OraStatement(this);
}

bool OraConnector::exec(const std::string & statement, SQLResult *& res, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	auto conn = priv->connectionManager.connection(ec);
	if (!conn)
		return false;

	std::unique_lock<std::mutex> __locker(conn->mut);

	SAS_LOG_VAR(priv->logger, statement);

	auto stmt = std::make_shared<OraStatement>(this);
	if (!stmt->prepare(statement, ec) ||
	    !stmt->exec(ec))
		return false;

	res = new OraResult(stmt);

	return true;
}

bool OraConnector::exec(const std::string & statement, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	auto conn = priv->connectionManager.connection(ec);
	if (!conn)
		return false;

	std::unique_lock<std::mutex> __locker(conn->mut);

	SAS_LOG_VAR(priv->logger, statement);

	auto stmt = std::make_shared<OraStatement>(this);
	if (!stmt->prepare(statement, ec) ||
		!stmt->execDML(ec))
		return false;

	return true;
}

void OraConnector::detach()
{
	priv->connectionManager.detach();
}

Logging::LoggerPtr OraConnector::logger() const
{
	return priv->logger;
}

const Oracle_Settings & OraConnector::settings() const
{
	return priv->settings;
}

std::mutex & OraConnector::mutex()
{
	auto conn = priv->connectionManager.connection();
	if (!conn)
	{
		SAS_LOG_NDC();
		SAS_LOG_ASSERT(priv->logger, conn, "connection is not yet activated");
	}
	return conn->mut;
}

bool OraConnector::activate(ErrorCollector & ec)
{
	SAS_LOG_NDC();
	return priv->connectionManager.connection(ec) != nullptr;
}

void OraConnector::lock()
{
	auto conn = priv->connectionManager.connection();
	if (!conn)
	{
		SAS_LOG_NDC();
		SAS_LOG_ASSERT(priv->logger, conn, "connection is not yet activated");
	}
	conn->external_mut.lock();
}

void OraConnector::unlock()
{
	auto conn = priv->connectionManager.connection();
	if (!conn)
	{
		SAS_LOG_NDC();
		SAS_LOG_ASSERT(priv->logger, conn, "connection is not yet activated");
	}
	conn->external_mut.unlock();
}

bool OraConnector::startTransaction(ErrorCollector & ec)
{
	SAS_LOG_NDC();
	return true;
}

bool OraConnector::commit(ErrorCollector & ec)
{
	SAS_LOG_NDC();
	auto conn = priv->connectionManager.connection();
	if(!conn)
	{
		auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "connection is not specified for the thread");
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}
	SAS_LOG_TRACE(priv->logger, "dpiConn_commit");
	if(!dpiConn_commit(conn->conn))
	{
		auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, getErrorText());
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}

	return true;
}

bool OraConnector::rollback(ErrorCollector & ec)
{
	SAS_LOG_NDC();
	auto conn = priv->connectionManager.connection();
	if(!conn)
	{
		auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "connection is not specified for the thread");
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}
	SAS_LOG_TRACE(priv->logger, "dpiConn_commit");
	if(!dpiConn_rollback(conn->conn))
	{
		auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, getErrorText());
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}

	return true;
}

dpiConn * OraConnector::conn(ErrorCollector & ec)
{
	auto c = priv->connectionManager.connection(ec);
	return c ? c->conn : nullptr;
}

dpiConn * OraConnector::conn()
{
	auto c = priv->connectionManager.connection();
	return c ? c->conn : nullptr;
}

dpiContext * OraConnector::ctx() const
{
	return priv->ctx;
}

std::string OraConnector::getErrorText()
{
	return OraTools::toString(priv->ctx);
}

}
