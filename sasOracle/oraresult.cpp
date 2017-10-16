/*
    This file is part of sasOracle.

    sasOracle is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasOracle is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasOracle.  If not, see <http://www.gnu.org/licenses/>
 */

#include "oraresult.h"
#include "orastatement.h"

#include <sasCore/errorcollector.h>
#include <sasCore/logging.h>

#include <assert.h>
#include <mutex>

namespace SAS {

struct OraResult::Priv
{
	Priv(const std::shared_ptr<OraStatement> & stmt_) : stmt(stmt_)
	{ }

	std::shared_ptr<OraStatement> stmt;
};

OraResult::OraResult(const std::shared_ptr<OraStatement> & stmt) : SQLResult(), priv(new Priv(stmt))
{
}

OraResult::~OraResult()
{
	delete priv;
}

bool OraResult::fieldNum(size_t & ret, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	return priv->stmt->fieldNum(ret, ec);
}

bool OraResult::fields(std::vector<std::tuple<std::string, std::string, std::string, SQLDataType>> & ret, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	return priv->stmt->fields(ret, ec);
}


unsigned long long OraResult::rowNum()
{
	SAS_LOG_NDC();
	return priv->stmt->rowNum();
}

bool OraResult::fetch(std::vector<SQLVariant> & ret, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	return priv->stmt->fetch(ret, ec);
}

}
