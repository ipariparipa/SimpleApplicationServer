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

#include <string>

#define SAS_OBJECT_TYPE__SQL_CONNECTOR "sql_connector"
#define SAS_OBJECT_TYPE_SQL__CONNECTOR SAS_OBJECT_TYPE__SQL_CONNECTOR

namespace SAS {

class ErrorCollector;
class SQLStatement;
class SQLResult;
class SQLDateTime;

class SAS_SQL__CLASS SQLConnector : public Object
{
protected:
	inline SQLConnector() { }
public:

	enum class Feature
	{
		GetServerInfo,
		MultiThreading,
		SimpleQuery,
		Transaction,
		GetSysDate,
		GetLastGeneratedId,
		Statement,
		BindingByPos,
		BindingByName
	};

	virtual inline ~SQLConnector() { }

	virtual inline std::string type() const final
			{ return SAS_OBJECT_TYPE_SQL__CONNECTOR; }

	bool hasFeature(Feature f)
	{
		std::string expl;
		return hasFeature(f, expl);
	}

	virtual bool getServerInfo(std::string & generation, std::string & version, ErrorCollector & ec) = 0;

	virtual const char * getServerType() const = 0;

	virtual bool hasFeature(Feature f, std::string & explanation) = 0;

	virtual bool connect(ErrorCollector & ec) = 0;

	virtual SQLStatement * createStatement(ErrorCollector & ec) = 0;

	virtual bool exec(const std::string & statement, SQLResult *& res, ErrorCollector & ec) = 0;
	virtual bool exec(const std::string & statement, ErrorCollector & ec) = 0;

	virtual void detach() = 0;

	virtual bool activate(ErrorCollector & ec) = 0;

	virtual void lock() = 0;
	virtual void unlock() = 0;

	virtual bool startTransaction(ErrorCollector & ec) = 0;
	virtual bool commit(ErrorCollector & ec) = 0;
	virtual bool rollback(ErrorCollector & ec) = 0;

	virtual bool getSysDate(SAS::SQLDateTime & ret, ErrorCollector & ec);
};


class SAS_SQL__CLASS SQLTransactionProtector
{
	SAS_COPY_PROTECTOR(SQLTransactionProtector)

	struct Priv;
	Priv * priv;
public:
	SQLTransactionProtector(SQLConnector * conn, bool auto_commit = false);
	~SQLTransactionProtector();

	bool commit(ErrorCollector & ec);
	bool rollback(ErrorCollector & ec);

};

}

#endif /* INCLUDE_SASSQL_SQLCONNECTOR_H_ */
