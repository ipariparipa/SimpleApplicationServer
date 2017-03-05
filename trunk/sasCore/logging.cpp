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

#include "include/sasCore/logging.h"

namespace SAS { namespace Logging {

std::string toString(const std::vector<std::string> & strl)
{
	std::string ret;
	for(size_t i = 0, l = strl.size(); i < l; ++i)
	{
		ret += strl[i];
		if(i+1 < l)
			ret += "; ";
	}
	return ret;
}

LoggerPtr getLogger(const std::string & name)
{
#ifdef SAS_LOG4CXX_ENABLED
	return log4cxx::Logger::getLogger(name);
#else
	return 0;
#endif
}

LoggerPtr getRootLogger()
{
#ifdef SAS_LOG4CXX_ENABLED
	return log4cxx::Logger::getRootLogger();
#else
	return 0;
#endif
}

}}


