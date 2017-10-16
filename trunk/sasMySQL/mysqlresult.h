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

#ifndef sasMySQL__mysqlresult_h
#define sasMySQL__mysqlresult_h

#include "config.h"
#include <sasSQL/sqlresult.h>
#include SAS_MYSQL__MYSQL_H

namespace SAS {

class MySQLConnector;

class MySQLResult : public SQLResult
{
	struct Priv;
	Priv * priv;
public:
	MySQLResult(MySQLConnector * conn, MYSQL_RES * res);
	virtual ~MySQLResult();

	virtual bool fieldNum(size_t & ret, ErrorCollector & ec) final;
	virtual bool fields(std::vector<std::tuple<std::string, std::string, std::string, SQLDataType>> & ret, ErrorCollector & ec) final;

	virtual unsigned long long rowNum() final;
	virtual bool fetch(std::vector<SQLVariant> &, ErrorCollector & ec) final;

	static SQLDataType toDataType(enum_field_types type);
};

}

#endif //sasMySQL__mysqlresult_h
