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

#ifndef INCLUDE_SASCORE_LOGGING_H_
#define INCLUDE_SASCORE_LOGGING_H_

#include "config.h"

#include <sstream>
#include <vector>

#ifdef SAS_LOG4CXX_ENABLED

#include <log4cxx/log4cxx.h>
#include <log4cxx/logger.h>
#include <log4cxx/ndc.h>

namespace SAS { namespace Logging {

	struct _NDC
	{
		_NDC(const char * msg)
		{
			log4cxx::NDC::push(msg);
		}

		~_NDC()
		{
			log4cxx::NDC::pop();
		}
	};

	typedef log4cxx::LoggerPtr LoggerPtr;

}}


#define SAS_LOG_NDC() SAS::Logging::_NDC __ndc__(__PRETTY_FUNCTION__)

#define SAS_LOG_TRACE(logger, msg) LOG4CXX_TRACE(logger, msg)
#define SAS_ROOT_LOG_TRACE(msg) SAS_LOG_TRACE(log4cxx::Logger::getRootLogger(), msg)

#define SAS_LOG_INFO(logger, msg) LOG4CXX_INFO(logger, msg)
#define SAS_ROOT_LOG_INFO(msg) SAS_LOG_INFO(log4cxx::Logger::getRootLogger(), msg)

#define SAS_LOG_DEBUG(logger, msg) LOG4CXX_DEBUG(logger, msg)
#define SAS_ROOT_LOG_DEBUG(msg) SAS_LOG_DEBUG(log4cxx::Logger::getRootLogger(), msg)

#define SAS_LOG_WARN(logger, msg) LOG4CXX_WARN(logger, msg)
#define SAS_ROOT_LOG_WARN(msg) SAS_LOG_WARN(log4cxx::Logger::getRootLogger(), msg)

#define SAS_LOG_ERROR(logger, msg) LOG4CXX_ERROR(logger, msg)
#define SAS_ROOT_LOG_ERROR(msg) SAS_LOG_ERROR(log4cxx::Logger::getRootLogger(), msg)

#define SAS_LOG_FATAL(logger, msg) LOG4CXX_FATAL(logger, msg)
#define SAS_ROOT_LOG_FATAL(msg) SAS_LOG_FATAL(log4cxx::Logger::getRootLogger(), msg)

#define SAS_LOG_ASSERT(logger, condition, msg) LOG4CXX_ASSERT(logger, condition, msg)
#define SAS_ROOT_LOG_ASSERT(condition, msg) SAS_LOG_ASSERT(log4cxx::Logger::getRootLogger(), conditnio, msg)

#define SAS_LOG_VAR_NAME(logger, var_name, val) SAS_LOG_DEBUG(logger, (std::string)var_name + " = " + SAS::Logging::toString(val))
#define SAS_ROOT_LOG_VAR_NAME(var_name, val) SAS_LOG_VAR_NAME(log4cxx::Logger::getRootLogger(), var_name, val)

#define SAS_LOG_VAR(logger, var) SAS_LOG_VAR_NAME(logger, #var, var)
#define SAS_ROOT_LOG_VAR(var) SAS_LOG_VAR(log4cxx::Logger::getRootLogger(), var)

#else

namespace SAS { namespace Logging {

typedef int LoggerPtr;

}}

#define SAS_LOG_NDC()

#define SAS_LOG_TRACE(logger, msg)
#define SAS_ROOT_LOG_TRACE(msg)

#define SAS_LOG_INFO(logger, msg)
#define SAS_ROOT_LOG_INFO(msg)

#define SAS_LOG_DEBUG(logger, msg)
#define SAS_ROOT_LOG_DEBUG(msg)

#define SAS_LOG_WARN(logger, msg)
#define SAS_ROOT_LOG_WARN(msg)

#define SAS_LOG_ERROR(logger, msg)
#define SAS_ROOT_LOG_ERROR(msg)

#define SAS_LOG_FATAL(logger, msg)
#define SAS_ROOT_LOG_FATAL(msg)

#define SAS_LOG_ASSERT(logger, condition, msg) assert(condition)
#define SAS_ROOT_LOG_ASSERT(condition, msg) assert(condition)

#define SAS_LOG_VAR_NAME(logger, var_name, val)
#define SAS_ROOT_LOG_VAR_NAME(var_name, val)

#define SAS_LOG_VAR(logger, var)
#define SAS_ROOT_LOG_VAR(var)

#endif

namespace SAS { namespace Logging {

LoggerPtr getLogger(const std::string & name);
LoggerPtr getRootLogger();

std::string toString(const std::vector<std::string> & strl);

template <typename T>
std::string toString(const T & v)
{
	std::stringstream ss;
	ss << v;
	return ss.str();
}

}}

#endif /* INCLUDE_SASCORE_LOGGING_H_ */
