/*
    This file is part of sasSQL.

    sasSQL is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasSQL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasSQL.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef INCLUDE_SASSQL_SQLSTATEMENT_H_
#define INCLUDE_SASSQL_SQLSTATEMENT_H_

#include "sqlresult.h"

#include <vector>
#include <string>

namespace SAS {

	class SAS_SQL__CLASS SQLStatement : public SQLResult
{
protected:
	inline SQLStatement() : SQLResult() { }
public:
	virtual inline ~SQLStatement() { }
	virtual bool prepare(const std::string & statement, ErrorCollector & ec) = 0;
	virtual unsigned long paramNum() = 0;
	virtual bool bindParam(const std::vector<SQLVariant> & params, ErrorCollector & ec) = 0;
	virtual bool bindParam(const std::vector<std::pair<std::string /*name*/, SQLVariant>> & params, ErrorCollector & ec) = 0;
	virtual bool execDML(ErrorCollector & ec) = 0;
	virtual bool exec(ErrorCollector & ec) = 0;
	virtual bool getLastGeneratedId(const std::string & schema, const std::string & table, const std::string & field, SQLVariant & ret, ErrorCollector & ec) = 0;

	virtual bool getSysDate(SAS::SQLDateTime & ret, ErrorCollector & ec) = 0;
};

}

#endif /* INCLUDE_SASSQL_SQLSTATEMENT_H_ */
