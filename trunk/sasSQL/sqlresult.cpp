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

#include "include/sasSQL/sqlresult.h"
#include "include/sasSQL/sqlstatement.h"

#include <sasCore/logging.h>

namespace SAS {

	struct SQLStatementResult::Priv
	{
		Priv(const std::shared_ptr<SQLStatement> & stmt_) : stmt(stmt_)
		{ }

		std::shared_ptr<SQLStatement> stmt;
	};

	SQLStatementResult::SQLStatementResult(const std::shared_ptr<SQLStatement> & stmt) : SQLResult(), priv(new Priv(stmt))
	{
	}

	SQLStatementResult::~SQLStatementResult()
	{
		delete priv;
	}

	bool SQLStatementResult::fieldNum(size_t & ret, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		return priv->stmt->fieldNum(ret, ec);
	}

	bool SQLStatementResult::fields(std::vector<std::tuple<std::string, std::string, std::string, SQLDataType>> & ret, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		return priv->stmt->fields(ret, ec);
	}

	unsigned long long SQLStatementResult::rowNum()
	{
		SAS_LOG_NDC();
		return priv->stmt->rowNum();
	}

	bool SQLStatementResult::fetch(std::vector<SQLVariant> & ret, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		return priv->stmt->fetch(ret, ec);
	}

}
