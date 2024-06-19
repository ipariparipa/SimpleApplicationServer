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
	virtual ~MySQLConnector() override;

	bool getServerInfo(std::string & generation, std::string & version, ErrorCollector & ec) final override;

	virtual inline const char * getServerType() const final override
	{ return "mysql"; }

	bool hasFeature(Feature f, std::string & explanation) final override;

	bool connect(ErrorCollector & ec) final override;

	SQLStatement * createStatement(ErrorCollector & ec) final override;

	std::string name() const final override;

	bool exec(const std::string & statement, SQLResult *& res, ErrorCollector & ec) final override;
	bool exec(const std::string & statement, ErrorCollector & ec) final override;

	void detach() final override;

	Logging::LoggerPtr logger() const;

	bool init(const std::string & configPath, ErrorCollector & ec) ;

	const MySQL_Settings & settings() const;

	std::mutex & mutex();

	bool activate(ErrorCollector & ec) final override;

	void lock() final override;
	void unlock() final override;

	bool startTransaction(ErrorCollector & ec) final override;
	bool commit(ErrorCollector & ec) final override;
	bool rollback(ErrorCollector & ec) final override;

protected:
	bool appendCompletionValue(const std::string& command, const std::vector<std::string>& args, std::string& ret, ErrorCollector& ec) const final override;
};

}

#endif // sasMySQL__mysqlconnector_h
