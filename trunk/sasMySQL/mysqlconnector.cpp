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

#include <mysql/mysql.h>

#include <sasCore/application.h>
#include <sasCore/configreader.h>

#include "mysqlresult.h"
#include "mysqlstatement.h"

#include <vector>
#include <mutex>

namespace SAS {

struct MySQLConnector_priv
{
	MySQLConnector_priv() : my(nullptr)
	{ }

	std::string name;
	struct ConnectionData
	{
		ConnectionData() : port(0), connected(false)
		{ }

		std::string host;
		std::string user;
		std::string passwd;
		std::string db;
		unsigned int port; // optional
		std::string unix_socket; // optional

		bool connected;
	} connectionData;

	Logging::LoggerPtr logger;

	Application * app;

	MYSQL * my;
	MySQL_Settings settings;

	std::mutex mut;
};


MySQLConnector::MySQLConnector(const std::string & name, Application * app) : SQLConnector(), priv(new MySQLConnector_priv)
{
	std::unique_lock<std::mutex> __locker(priv->mut);
	priv->my = mysql_init(nullptr);
	priv->app = app;
	priv->logger = Logging::getLogger("SAS.MySQLConnector." + name);
	priv->name= name;
}

MySQLConnector::~MySQLConnector()
{
	std::unique_lock<std::mutex> __locker(priv->mut);
	mysql_close(priv->my);
	delete priv;
}

std::string MySQLConnector::name() const
{ return priv->name; }

bool MySQLConnector::init(const std::string & configPath, ErrorCollector & ec)
{
	std::unique_lock<std::mutex> __locker(priv->mut);
	SAS_LOG_NDC();
	auto cfg = priv->app->configreader();

	bool has_error(false);

	SAS_LOG_TRACE(logger(), "read connection data from config");
	if(!cfg->getStringEntry(configPath + "/HOST", priv->connectionData.host, ec))
		has_error = true;
	if(!cfg->getStringEntry(configPath + "/USER", priv->connectionData.user, ec))
		has_error = true;
	if(!cfg->getStringEntry(configPath + "/PASSWD", priv->connectionData.passwd, ec))
		has_error = true;
	if(!cfg->getStringEntry(configPath + "/DB", priv->connectionData.db, ec))
		has_error = true;
	long long ll_tmp;
	if(!cfg->getNumberEntry(configPath + "/MAX_BUFFER_SIZE", ll_tmp, 33554432, ec)) //32M
		has_error = true;
	else
	{
		priv->settings.max_buffer_size = ll_tmp;
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
		priv->connectionData.port = ll_tmp;
	cfg->getStringEntry(configPath + "/UNIX_SOCKET", priv->connectionData.unix_socket, priv->connectionData.unix_socket, ec);

	if(has_error)
	{
		SAS_LOG_DEBUG(logger(), "connection data are not complete");
		return false;
	}

	return true;
}

bool MySQLConnector::connect(ErrorCollector & ec)
{
	std::unique_lock<std::mutex> __locker(priv->mut);
	SAS_LOG_NDC();

	if(priv->connectionData.connected)
	{
		SAS_LOG_WARN(logger(), "MySQL connection had been already built");
		return true;
	}

	SAS_LOG_VAR(logger(), priv->connectionData.host);
	SAS_LOG_VAR(logger(), priv->connectionData.user);
	SAS_LOG_VAR(logger(), priv->connectionData.passwd);
	SAS_LOG_VAR(logger(), priv->connectionData.db);
	SAS_LOG_VAR(logger(), priv->connectionData.port);
	SAS_LOG_VAR(logger(), priv->connectionData.unix_socket);

	SAS_LOG_TRACE(logger(), "mysql_real_connect");
	if(!mysql_real_connect(priv->my,
			priv->connectionData.host.c_str(),
			priv->connectionData.user.c_str(),
			priv->connectionData.passwd.c_str(),
			priv->connectionData.db.c_str(),
			priv->connectionData.port,
			priv->connectionData.unix_socket.size() ? priv->connectionData.unix_socket.c_str() : NULL,
			0))
	{
		auto err = ec.add(-1, std::string("could not connect to MySQL service: ") + mysql_error(priv->my));
		SAS_LOG_ERROR(logger(), err);
		return false;
	}
	priv->connectionData.connected = true;
	SAS_LOG_DEBUG(logger(), "server connection has been successfully built");
	return true;
}

SQLStatement * MySQLConnector::createStatement()
{
	std::unique_lock<std::mutex> __locker(priv->mut);
	SAS_LOG_NDC();

	SAS_LOG_TRACE(logger(), "mysql_stmt_init");
	MYSQL_STMT * stmt = mysql_stmt_init(priv->my);
	return new MySQLStatement(stmt, this);
}

bool MySQLConnector::exec(const std::string & statement, SQLResult *& res, ErrorCollector & ec)
{
	std::unique_lock<std::mutex> __locker(priv->mut);
	SAS_LOG_NDC();

	SAS_LOG_VAR(logger(), statement);

	SAS_LOG_TRACE(logger(), "mysql_query");
	if(mysql_query(priv->my, statement.c_str()))
	{
		auto err = ec.add(-1, std::string("could execute SQL statement: ") + mysql_error(priv->my));
		SAS_LOG_ERROR(logger(), err);
		return false;
	}

	SAS_LOG_TRACE(logger(), "mysql_store_result");
	res = new MySQLResult(this, mysql_store_result(priv->my));

	return true;
}

bool MySQLConnector::exec(const std::string & statement, ErrorCollector & ec)
{
	std::unique_lock<std::mutex> __locker(priv->mut);
	SAS_LOG_NDC();

	SAS_LOG_VAR(logger(), statement);

	SAS_LOG_TRACE(logger(), "mysql_query");
	if(mysql_query(priv->my, statement.c_str()))
	{
		auto err = ec.add(-1, std::string("could execute SQL statement: ") + mysql_error(priv->my));
		SAS_LOG_ERROR(logger(), err);
		return false;
	}

	return true;
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
	return priv->mut;
}

}




