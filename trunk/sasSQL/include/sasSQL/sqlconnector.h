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

#ifndef INCLUDE_SASSQL_SQLCONNECTOR_H_
#define INCLUDE_SASSQL_SQLCONNECTOR_H_

#include "config.h"
#include <sasCore/object.h>

#define SAS_OBJECT_TYPE_SQL__CONNECTOR "sql_connector"

namespace SAS {

class ErrorCollector;
class SQLStatement;
class SQLResult;

class SAS_SQL__CLASS SQLConnector : public Object
{
protected:
	inline SQLConnector() { }
public:
	virtual inline ~SQLConnector() { }

	virtual inline std::string type() const final
			{ return SAS_OBJECT_TYPE_SQL__CONNECTOR; }

	virtual bool connect(ErrorCollector & ec) = 0;

	virtual SQLStatement * createStatement(ErrorCollector & ec) = 0;

	virtual bool exec(const std::string & statement, SQLResult *& res, ErrorCollector & ec) = 0;
	virtual bool exec(const std::string & statement, ErrorCollector & ec) = 0;

	virtual void detach() = 0;
};

}

#endif /* INCLUDE_SASSQL_SQLCONNECTOR_H_ */
