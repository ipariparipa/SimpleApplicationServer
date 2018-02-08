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

#ifndef sasODBC__odbcstatement_h
#define sasODBC__odbcstatement_h

#include "config.h"
#include <sasSQL/sqlstatement.h>
#include "include_odbc.h"

namespace SAS {

class ODBCConnector;

class ODBCStatement : public SQLStatement
{
	struct Priv;
	Priv * priv;
public:
	ODBCStatement(ODBCConnector * conn);
	virtual ~ODBCStatement();

	bool init(ErrorCollector & ec);

	virtual bool prepare(const std::string & statement, ErrorCollector & ec) final;
	virtual unsigned long paramNum() final;
	virtual bool bindParam(const std::vector<SQLVariant> & params, ErrorCollector & ec) final;
	virtual bool bindParam(const std::vector<std::pair<std::string /*name*/, SQLVariant>> & params, ErrorCollector & ec) final;
	virtual bool execDML(ErrorCollector & ec) final;
	virtual bool exec(ErrorCollector & ec) final;
	virtual bool getLastGeneratedId(const std::string & schema, const std::string & table, const std::string & field, SQLVariant & ret, ErrorCollector & ec) final;

	virtual bool fieldNum(size_t & ret, ErrorCollector & ec) final;
	virtual bool fields(std::vector<std::tuple<std::string /*db/scheme*/, std::string /*table*/, std::string /*field name*/, SQLDataType>> & ret, ErrorCollector & ec) final;
	virtual unsigned long long rowNum() final;
	virtual bool fetch(std::vector<SQLVariant> &, ErrorCollector & ec) final;

	virtual bool getSysDate(SAS::SQLDateTime & ret, ErrorCollector & ec) final;

};

}

#endif // sasODBC__odbcstatement_h
