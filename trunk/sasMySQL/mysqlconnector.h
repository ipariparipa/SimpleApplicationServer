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

#ifndef sasMySQL__mysqlconnector_h
#define sasMySQL__mysqlconnector_h

#include "config.h"
#include <sasSQL/sqlconnector.h>
#include <sasCore/logging.h>

#include <mutex>

namespace SAS {

class Application;

struct MySQL_Settings
{
	inline MySQL_Settings() : max_buffer_size(0), max_connections(0)
	{ }

	size_t max_buffer_size;
	size_t max_connections;
};

class MySQLConnector : public SQLConnector
{
	struct Priv;
	Priv * priv;
public:
	MySQLConnector(const std::string & name, Application * app);
	virtual ~MySQLConnector();

	bool init(const std::string & configPath, ErrorCollector & ec);

	virtual bool connect(ErrorCollector & ec) final;

	virtual SQLStatement * createStatement(ErrorCollector & ec) final;

	virtual std::string name() const final;

	virtual bool exec(const std::string & statement, SQLResult *& res, ErrorCollector & ec) final;
	virtual bool exec(const std::string & statement, ErrorCollector & ec) final;

	virtual void detach() final;

	Logging::LoggerPtr logger() const;

	const MySQL_Settings & settings() const;

	std::mutex & mutex();

	virtual bool activate(ErrorCollector & ec) final;

	virtual void lock() final;
	virtual void unlock() final;

	virtual bool startTransaction(ErrorCollector & ec) final;
	virtual bool commit(ErrorCollector & ec) final;
	virtual bool rollback(ErrorCollector & ec) final;
};

}

#endif // sasMySQL__mysqlconnector_h
