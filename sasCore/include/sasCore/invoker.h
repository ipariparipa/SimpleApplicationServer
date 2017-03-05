/*
    This file is part of sasCore.

    sasCore is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasCore is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasCore.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef INCLUDE_SASCORE_INVOKER_H_
#define INCLUDE_SASCORE_INVOKER_H_

#include <vector>
#include "defines.h"

namespace SAS
{

class ErrorCollector;

struct Invoker_priv;
class SAS_CORE__CLASS Invoker
{
public:
	Invoker();
	virtual ~Invoker();

	enum class Status
	{
		OK, Error, FatalError, NotImplemented
	};

	virtual Status invoke(const std::vector<char> & input, std::vector<char> & output, ErrorCollector & ec) = 0;

private:
	Invoker_priv * priv;
};

}

#endif /* INCLUDE_SASCORE_INVOKER_H_ */
