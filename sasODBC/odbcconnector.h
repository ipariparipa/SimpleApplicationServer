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

#ifndef sasODBC__odbcconnector_h
#define sasODBC__odbcconnector_h

#include "config.h"
#include <sasSQL/sqlconnector.h>
#include <sasCore/logging.h>
#include "include_odbc.h"

#include <mutex>

namespace SAS {

class Application;

struct ODBC_Settings
{
	size_t max_buffer_size = 0;
	size_t max_connections = 0;
	bool long_long_bind_supported = true;

	struct Info
	{
		int dtprec = 0;
		bool transaction_support = false;
		std::string db_type;
	} info;
};

class ODBCConnector : public SQLConnector
{
	SAS_COPY_PROTECTOR(ODBCConnector)

	struct Priv;
	Priv * priv;
public:
	ODBCConnector(SQLHENV env, const std::string & name, Application * app);
	virtual ~ODBCConnector();

	virtual const char * getServerType() const final;

	virtual bool getServerInfo(std::string & generation, std::string & version, ErrorCollector & ec) final;

	virtual bool hasFeature(Feature f, std::string & explanation) final;

	bool init(const std::string & configPath, ErrorCollector & ec);

	virtual bool connect(ErrorCollector & ec) final;

	virtual SQLStatement * createStatement(ErrorCollector & ec) final;

	virtual std::string name() const final;

	virtual bool exec(const std::string & statement, SQLResult *& res, ErrorCollector & ec) final;
	virtual bool exec(const std::string & statement, ErrorCollector & ec) final;

	virtual void detach() final;

	Logging::LoggerPtr logger() const;

	const ODBC_Settings & settings() const;

	std::mutex & mutex();

	virtual bool activate(ErrorCollector & ec) final;

	virtual void lock() final;
	virtual void unlock() final;

	virtual bool startTransaction(ErrorCollector & ec) final;
	virtual bool commit(ErrorCollector & ec) final;
	virtual bool rollback(ErrorCollector & ec) final;

	SQLHDBC conn();
	SQLHDBC conn(ErrorCollector & ec);
	SQLHENV env() const;

	std::string getErrorText(SQLHSTMT stmt, SQLRETURN rc, ErrorCollector & ec);

	std::string getErrorText();

};

}

#endif // sasODBC__odbcconnector_h
