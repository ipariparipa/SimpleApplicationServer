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
#include "defines.h"

#include <sstream>
#include <vector>

#ifdef SAS_LOG4CXX_ENABLED

#include <log4cxx/log4cxx.h>
#include <log4cxx/logger.h>
#include <log4cxx/ndc.h>
#include <assert.h>

namespace SAS { namespace Logging {

	struct SAS_CORE__CLASS _NDC
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

#define SAS_LOG_ASSERT(logger, condition, msg) LOG4CXX_ASSERT(logger, condition, msg); assert(condition)
#define SAS_ROOT_LOG_ASSERT(condition, msg) SAS_LOG_ASSERT(log4cxx::Logger::getRootLogger(), conditnio, msg)

#define SAS_LOG_VAR_NAME(logger, var_name, val) SAS_LOG_DEBUG(logger, (std::string)var_name + " = " + SAS::Logging::toString(val))
#define SAS_ROOT_LOG_VAR_NAME(var_name, val) SAS_LOG_VAR_NAME(log4cxx::Logger::getRootLogger(), var_name, val)

#define SAS_LOG_VAR(logger, var) SAS_LOG_VAR_NAME(logger, #var, var)
#define SAS_ROOT_LOG_VAR(var) SAS_LOG_VAR(log4cxx::Logger::getRootLogger(), var)

#else

#ifndef _M_CEE
#include <map>
#include <mutex>
#endif

#include <assert.h>

#include <sstream>

namespace SAS { namespace Logging {

class SAS_CORE__CLASS AbstractLogger
{
public:
	virtual inline ~AbstractLogger() { }

	enum class Priority : int
	{
		Trace, Debug, Info, Warn, Error, Fatal
	};

	virtual bool isEnabled(Priority prio) = 0;

	virtual void add(Priority prio, const std::string & message, const char * file = nullptr, long line = 0) = 0;
};

typedef AbstractLogger * LoggerPtr;

class SAS_CORE__CLASS AbstractLogging
{
public:
	virtual inline ~AbstractLogging() { }

	virtual LoggerPtr getLogger(const std::string & logger_name) = 0;
	virtual LoggerPtr getRootLogger() = 0;
};


struct SimpleLogger_priv;
class SAS_CORE__CLASS SimpleLogger : public AbstractLogger
{
	SAS_COPY_PROTECTOR(SimpleLogger)
public:
	SimpleLogger(const std::string & name, Priority min_prio);
	virtual ~SimpleLogger();

	virtual bool isEnabled(Priority prio) override;

	virtual void add(Priority prio, const std::string & message, const char * file = nullptr, long line = 0) override;

protected:
	virtual void add(const std::string & text) = 0;

private:
	SimpleLogger_priv * priv;
};

template <class Stream_T>
class StreamLogger : public SimpleLogger
{
public:
	inline StreamLogger(Stream_T & stream_, const std::string & name, Priority min_prio) :
		SimpleLogger(name, min_prio),
		stream(stream_)
	{ };

protected:
	virtual void add(const std::string & text) override
	{
		std::stringstream ss;
		ss << text << std::endl;
		stream << ss.str();
	}

private:
	Stream_T & stream;
};

#ifndef _M_CEE
template<class Stream_T>
class StreamLogging : public AbstractLogging
{
public:
	inline StreamLogging(Stream_T & stream_, AbstractLogger::Priority min_prio_) :
		AbstractLogging(),
		stream(stream_), min_prio(min_prio_), root(stream_, "root", min_prio_)
	{ }

	virtual LoggerPtr getLogger(const std::string & logger_name) override
	{
		std::unique_lock<std::mutex> __locker(mut);
		auto & ret = logger_reg[logger_name];
		if(ret)
			return ret;
		return ret = new StreamLogger<Stream_T>(stream, logger_name, min_prio);
	}

	virtual inline LoggerPtr getRootLogger() override
	{
		return &root;
	}

private:
	Stream_T & stream;
	AbstractLogger::Priority min_prio;
	StreamLogger<Stream_T> root;

	std::mutex mut;
	std::map<std::string, LoggerPtr> logger_reg;
};
#endif

extern SAS_CORE__FUNCTION void setLogging(AbstractLogging * logging);

}}

#define SAS_LOG_NDC()

#define SAS_LOG_ADD(logger, prio, msg) if(logger->isEnabled(prio)) logger->add(prio, msg, __FILE__, __LINE__)

#define SAS_LOG_TRACE(logger, msg) SAS_LOG_ADD(logger, SAS::Logging::AbstractLogger::Priority::Trace, msg)
#define SAS_ROOT_LOG_TRACE(msg) SAS_LOG_TRACE(SAS::Logging::getRootLogger(), msg)

#define SAS_LOG_INFO(logger, msg) SAS_LOG_ADD(logger, SAS::Logging::AbstractLogger::Priority::Info, msg)
#define SAS_ROOT_LOG_INFO(msg) SAS_LOG_INFO(SAS::Logging::getRootLogger(), msg)

#define SAS_LOG_DEBUG(logger, msg) SAS_LOG_ADD(logger, SAS::Logging::AbstractLogger::Priority::Debug, msg)
#define SAS_ROOT_LOG_DEBUG(msg) SAS_LOG_DEBUG(SAS::Logging::getRootLogger(), msg)

#define SAS_LOG_WARN(logger, msg) SAS_LOG_ADD(logger, SAS::Logging::AbstractLogger::Priority::Warn, msg)
#define SAS_ROOT_LOG_WARN(msg) SAS_LOG_WARN(SAS::Logging::getRootLogger(), msg)

#define SAS_LOG_ERROR(logger, msg) SAS_LOG_ADD(logger, SAS::Logging::AbstractLogger::Priority::Error, msg)
#define SAS_ROOT_LOG_ERROR(msg) SAS_LOG_ERROR(SAS::Logging::getRootLogger(), msg)

#define SAS_LOG_FATAL(logger, msg) SAS_LOG_ADD(logger, SAS::Logging::AbstractLogger::Priority::Fatal, msg)
#define SAS_ROOT_LOG_FATAL(msg) SAS_LOG_FATAL(SAS::Logging::getRootLogger(), msg)

#define SAS_LOG_ASSERT(logger, condition, msg) \
		if(!(condition)) \
			logger->add(SAS::Logging::AbstractLogger::Priority::Fatal, std::string(#condition) + " " + msg, __FILE__, __LINE__); \
		assert(condition)
#define SAS_ROOT_LOG_ASSERT(condition, msg) assert(condition) SAS_ROOT_LOG_ASSERT(SAS::Logging::getRootLogger(), condition, msg)

#define SAS_LOG_VAR_NAME(logger, var_name, val) SAS_LOG_DEBUG(logger, (std::string)var_name + " = " + SAS::Logging::toString(val))
#define SAS_ROOT_LOG_VAR_NAME(var_name, val) SAS_LOG_VAR_NAME(SAS::Logging::getRootLogger(), var_name, val)

#define SAS_LOG_VAR(logger, var) SAS_LOG_VAR_NAME(logger, #var, var)
#define SAS_ROOT_LOG_VAR(var) SAS_LOG_VAR(SAS::Logging::getRootLogger(), var)

#endif

namespace SAS { namespace Logging {

	extern SAS_CORE__FUNCTION LoggerPtr getLogger(const std::string & name);
	extern SAS_CORE__FUNCTION LoggerPtr getRootLogger();

	extern SAS_CORE__FUNCTION std::string toString(const std::vector<std::string> & strl);

	template <typename T>
	std::string toString(const T & v)
	{
		std::stringstream ss;
		ss << v;
		return ss.str();
	}

}}

#endif /* INCLUDE_SASCORE_LOGGING_H_ */
