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
    along with ${project_name}.  If not, see <http://www.gnu.org/licenses/>
 */

#include "include/sasCore/errorcollector.h"
#include <sstream>

namespace SAS {

SimpleErrorCollector::SimpleErrorCollector(std::function<void(long errorCode, const std::string & errorText)> fnct) : _fnct(fnct)
{ }

std::string SimpleErrorCollector::add(long errorCode, const std::string & errorText)
{
	_fnct(errorCode, errorText);
	std::stringstream ss;
	ss << "[" << errorCode << "] " << errorText;
	return ss.str();
}

}



