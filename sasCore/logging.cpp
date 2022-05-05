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

#include "include/sasCore/logging.h"
#include "include/sasCore/thread.h"

#include <iostream>
#include <mutex>
#include <sstream>
#include <iomanip>

namespace SAS { namespace Logging {

	extern SAS_CORE__FUNCTION std::string toString(const std::vector<std::string> & strl)
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

#ifdef SAS_LOG4CXX_ENABLED
#else

	struct SimpleLogger_priv
	{
		SimpleLogger_priv(const std::string & name_, AbstractLogger::Priority min_prio_) :
			name(name_), min_prio(min_prio_)
		{ }

		std::string name;
		AbstractLogger::Priority min_prio;
	};

	SimpleLogger::SimpleLogger(const std::string & name, Priority min_prio) :
			priv(new SimpleLogger_priv(name, min_prio))
	{ }

	SimpleLogger::~SimpleLogger()
	{
		delete priv;
	}

	bool SimpleLogger::isEnabled(Priority prio)
	{
		return prio >= priv->min_prio;
	}

	void SimpleLogger::add(Priority prio, const std::string & message, const char * file, long line)
	{
		if(!isEnabled(prio))
			return;
		std::stringstream ss;

        auto now = std::chrono::system_clock::now();
        std::time_t tt = std::chrono::system_clock::to_time_t(now);
        std::tm tm = *std::localtime(&tt); //UTC: gmtime
        auto part = now.time_since_epoch().count() - (std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count() * std::chrono::system_clock::duration::period::den);

        ss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S") << "." << part << " " << Thread::getThreadId() << " [";
		switch(prio)
		{
		case Priority::Trace: ss << "TRACE"; break;
		case Priority::Debug: ss << "DEBUG"; break;
		case Priority::Info:  ss << "INFO"; break;
		case Priority::Warn:  ss << "WARN"; break;
		case Priority::Error: ss << "ERROR"; break;
		case Priority::Fatal: ss << "FATAL"; break;
		}
		ss << "] - " << priv->name << " - " << message;
		if(file)
			ss << " {"<<file<<':'<<line<<"}";

		add(ss.str());
	}

	std::mutex logging_mut;
	AbstractLogging * logging = nullptr;

	extern SAS_CORE__FUNCTION void setLogging(AbstractLogging * logging_)
	{
		std::unique_lock<std::mutex> __locker(logging_mut);
		logging = logging_;
	}

#endif


	extern SAS_CORE__FUNCTION LoggerPtr getLogger(const std::string & name)
	{
#ifdef SAS_LOG4CXX_ENABLED
		return log4cxx::Logger::getLogger(name);
#else
		std::unique_lock<std::mutex> __locker(logging_mut);
		return (logging ? logging : (logging = new StreamLogging<std::ostream>(std::cout, AbstractLogger::Priority::Fatal)))->getLogger(name);
#endif
	}

	extern SAS_CORE__FUNCTION LoggerPtr getRootLogger()
	{
#ifdef SAS_LOG4CXX_ENABLED
		return log4cxx::Logger::getRootLogger();
#else
		std::unique_lock<std::mutex> __locker(logging_mut);
		return (logging ? logging : (logging = new StreamLogging<std::ostream>(std::cout, AbstractLogger::Priority::Fatal)))->getRootLogger();
#endif
	}

}}
