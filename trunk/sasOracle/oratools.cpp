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

#include "oratools.h"

#include <sstream>

namespace SAS { namespace OraTools {

	std::string toString(const dpiErrorInfo & ei)
	{
		std::string _msg;
		_msg.append(ei.message, ei.messageLength);
		std::stringstream ss;
		ss << "[" << ei.code << "]" << _msg << " (" << ei.fnName << ": " << ei.action << ")";
		return ss.str();
	}

	std::string toString(dpiContext * ctx)
	{
		dpiErrorInfo ei;
		dpiContext_getError(ctx, &ei);
		return toString(ei);
	}

		
}}
