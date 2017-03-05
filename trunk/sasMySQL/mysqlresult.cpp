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

#include "mysqlresult.h"
#include "mysqlconnector.h"

#include <sasCore/errorcollector.h>

#include <assert.h>

namespace SAS {

struct MySQLResult_priv
{
	MySQLConnector * conn;
	MYSQL_RES * res;
};

MySQLResult::MySQLResult(MySQLConnector * conn, MYSQL_RES * res) : SQLResult(), priv(new MySQLResult_priv)
{
	priv->conn = conn;
	priv->res = res;
}

MySQLResult::~MySQLResult()
{
	mysql_free_result(priv->res);
	delete priv;
}

bool MySQLResult::fieldNum(size_t & ret, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	SAS_LOG_TRACE(priv->conn->logger(), "mysql_num_fields");
	ret = mysql_num_fields(priv->res);
	return true;
}

bool MySQLResult::fields(std::vector<std::tuple<std::string, std::string, std::string, SQLDataType>> & ret, ErrorCollector & ec)
{
	SAS_LOG_NDC();

	SAS_LOG_TRACE(priv->conn->logger(), "mysql_num_fields");
	unsigned long field_num = mysql_num_fields(priv->res);
	SAS_LOG_VAR(priv->conn->logger(), field_num);
	ret.resize(field_num);

	MYSQL_FIELD * f;
	for(unsigned int i = 0; i < field_num; ++i)
	{
		SAS_LOG_TRACE(priv->conn->logger(), "mysql_fetch_field_direct");
		assert(f = mysql_fetch_field_direct(priv->res, i));
		ret[i] = std::tuple<std::string, std::string, std::string, SQLDataType>(
			f->db ? f->db : std::string(),
			f->table ? f->table : std::string(),
			f->name ? f->name : std::string(),
			toDataType(f->type)
		);
	}

	return true;
}

//static
SQLDataType MySQLResult::toDataType(enum_field_types type)
{
	switch(type)
	{
	case MYSQL_TYPE_NULL:
		break;
	case MYSQL_TYPE_TINY:
	case MYSQL_TYPE_LONG:
	case MYSQL_TYPE_INT24:
	case MYSQL_TYPE_LONGLONG:
		return SQLDataType::Number;
	case MYSQL_TYPE_FLOAT:
	case MYSQL_TYPE_DOUBLE:
		return SQLDataType::Real;
	case MYSQL_TYPE_VARCHAR:
	case MYSQL_TYPE_VAR_STRING:
	case MYSQL_TYPE_STRING:
	case MYSQL_TYPE_TINY_BLOB:
	case MYSQL_TYPE_MEDIUM_BLOB:
	case MYSQL_TYPE_LONG_BLOB:
	case MYSQL_TYPE_BLOB:
	case MYSQL_TYPE_BIT:
	case MYSQL_TYPE_NEWDECIMAL:
	case MYSQL_TYPE_DECIMAL:
		return SQLDataType::String;
	case MYSQL_TYPE_YEAR:
	case MYSQL_TYPE_SHORT:
		return SQLDataType::Number;
	case MYSQL_TYPE_NEWDATE:
	case MYSQL_TYPE_DATE:
	case MYSQL_TYPE_TIME:
	case MYSQL_TYPE_TIME2:
	case MYSQL_TYPE_TIMESTAMP:
	case MYSQL_TYPE_TIMESTAMP2:
	case MYSQL_TYPE_DATETIME:
	case MYSQL_TYPE_DATETIME2:
		return SQLDataType::DateTime;
	case MYSQL_TYPE_JSON:
	case MYSQL_TYPE_ENUM:
	case MYSQL_TYPE_SET:
	case MYSQL_TYPE_GEOMETRY:
		break;
	}
	return SQLDataType::None;
}

unsigned long long MySQLResult::rowNum()
{
	return mysql_num_rows(priv->res);
}

bool MySQLResult::fetch(std::vector<SQLVariant> & ret, ErrorCollector & ec)
{
	SAS_LOG_NDC();

	MYSQL_ROW row;
	SAS_LOG_TRACE(priv->conn->logger(), "mysql_fetch_row");
	if(!(row = mysql_fetch_row(priv->res)))
	{
		SAS_LOG_TRACE(priv->conn->logger(), "could not fetch next row");
		return false;
	}

	SAS_LOG_TRACE(priv->conn->logger(), "mysql_num_fields");
	unsigned long field_num = mysql_num_fields(priv->res);
	SAS_LOG_VAR(priv->conn->logger(), field_num);
	ret.resize(field_num);
	for(unsigned long i = 0; i < field_num; ++i)
		if(row[i])
			ret[i] = SQLVariant((const char *)row[i]);
		else
			ret[i] = SQLVariant(SQLDataType::String);

	return true;
}


}

