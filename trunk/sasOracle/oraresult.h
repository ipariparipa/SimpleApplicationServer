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

#ifndef sasOracle__oraresult_h
#define sasOracle__oraresult_h

#include "config.h"
#include <sasSQL/sqlresult.h>
#include SAS_ORACLE__DPI_H

#include <memory>

namespace SAS {

	class OraStatement;

class OraResult : public SQLResult
{
	struct Priv;
	Priv * priv;
public:
	OraResult(const std::shared_ptr<OraStatement> & stmt);
	virtual ~OraResult();

	virtual bool fieldNum(size_t & ret, ErrorCollector & ec) final;
	virtual bool fields(std::vector<std::tuple<std::string /*db*/, std::string /*table*/, std::string /*name*/, SQLDataType>> & ret, ErrorCollector & ec) final;

	virtual unsigned long long rowNum() final;
	virtual bool fetch(std::vector<SQLVariant> &, ErrorCollector & ec) final;

};

}

#endif // sasOracle__oraresult_h
