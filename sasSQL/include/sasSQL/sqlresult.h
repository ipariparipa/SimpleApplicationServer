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

#ifndef INCLUDE_SASSQL_SQLRESULT_H_
#define INCLUDE_SASSQL_SQLRESULT_H_

#include "config.h"
#include <sasCore/defines.h>
#include "sqlvariant.h"

#include <vector>
#include <tuple>
#include <string>
#include <memory>

namespace SAS {

class ErrorCollector;

class SAS_SQL__CLASS SQLResult
{
protected:
	inline SQLResult() { }
public:
	virtual inline ~SQLResult() { }

	virtual bool fieldNum(size_t & ret, ErrorCollector & ec) = 0;
	virtual bool fields(std::vector<std::tuple<std::string /*db/scheme*/, std::string /*table*/, std::string /*field name*/, SQLDataType>> & ret, ErrorCollector & ec) = 0;
	virtual unsigned long long rowNum() = 0;
	virtual bool fetch(std::vector<SQLVariant> &, ErrorCollector & ec) = 0;
};

class SQLStatement;

class SAS_SQL__CLASS SQLStatementResult : public SQLResult
{
	SAS_COPY_PROTECTOR(SQLStatementResult)

	struct Priv;
	Priv * priv;
public:
	SQLStatementResult(const std::shared_ptr<SQLStatement> & stmt);
	virtual inline ~SQLStatementResult();

	virtual bool fieldNum(size_t & ret, ErrorCollector & ec) final;
	virtual bool fields(std::vector<std::tuple<std::string /*db/scheme*/, std::string /*table*/, std::string /*field name*/, SQLDataType>> & ret, ErrorCollector & ec) final;
	virtual unsigned long long rowNum() final;
	virtual bool fetch(std::vector<SQLVariant> &, ErrorCollector & ec) final;
};

}

#endif /* INCLUDE_SASSQL_SQLRESULT_H_ */
