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

#ifndef sasOracle__oratools_h
#define sasOracle__oratools_h

#include "config.h"
#include <string>
#include SAS_ORACLE__DPI_H

namespace SAS { namespace OraTools {

	std::string toString(const dpiErrorInfo & ei);
	std::string toString(dpiContext * ctx);

}}

#endif // sasOracle__oratools_h
