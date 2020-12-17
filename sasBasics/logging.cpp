/*
    This file is part of sasBasics.

    sasBasics is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasBasics is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasBasics.  If not, see <http://www.gnu.org/licenses/>
 */

#include "include/sasBasics/logging.h"
#include "include/sasBasics/errorcodes.h"

#include <sasCore/errorcollector.h>

#ifdef SAS_LOG4CXX_ENABLED
#  include <log4cxx/basicconfigurator.h>
#  include <log4cxx/xml/domconfigurator.h>
#  include <log4cxx/propertyconfigurator.h>
#  include <log4cxx/consoleappender.h>
#  include <log4cxx/patternlayout.h>
#  include <log4cxx/xml/xmllayout.h>
#  include <log4cxx/fileappender.h>
#  include <log4cxx/simplelayout.h>
#  include <log4cxx/htmllayout.h>
#else
#  include <iostream>
#  include <fstream>
#endif


namespace SAS { namespace Logging {

extern SAS_BASICS__FUNCTION bool init(int argc, char *argv[], ErrorCollector & ec)
{
#ifdef SAS_LOG4CXX_ENABLED
    (void)ec;
    enum State
	{
		None,
		XMLConfig,
        PropertyConfig,
        Level,
        Pattern,
        Appender,
        Layout,
        Filename,
        Target
    } st(None);

    bool basic_config = true;

    log4cxx::AppenderPtr appender;
    log4cxx::LayoutPtr layout;
    log4cxx::LevelPtr level;

	for(int i = 0; i < argc ; ++i)
	{
        std::string arg=argv[i];
		switch(st)
		{
		case None:
			if(st == None)
			{
                if(arg == "-log4cxx-xml-config")
					st = XMLConfig;
                else if(arg == "-log4cxx-property-config")
					st = PropertyConfig;
                else if(arg == "-log4cxx-basic-config")
                    layout = new log4cxx::PatternLayout("%t [%-5p] - %c - %m {%F:%L}%n");
                else if(arg == "-log4cxx-appender")
                    st = Appender;
                else if(arg == "-log4cxx-layout")
                    st = Layout;
                else if(arg == "-log4cxx-level")
                    st = Level;
            }
			break;
		case XMLConfig:
            log4cxx::xml::DOMConfigurator::configure(arg);
			st = None;
            basic_config = false;
			break;
		case PropertyConfig:
            st = None;
            log4cxx::PropertyConfigurator::configure(arg);
            basic_config = false;
            break;
        case Level:
            st = None;
            if(arg == "trace")
                level = log4cxx::Level::getTrace();
            else if(arg == "debug")
                level = log4cxx::Level::getDebug();
            else if(arg == "info")
                level = log4cxx::Level::getInfo();
            else if(arg == "warn")
                level = log4cxx::Level::getWarn();
            else if(arg == "error")
                level = log4cxx::Level::getError();
            else if(arg == "fatal")
                level = log4cxx::Level::getFatal();
            else if(arg == "off")
                level = log4cxx::Level::getOff();
            else if(arg == "all")
                level = log4cxx::Level::getAll();
            else
            {
                ec.add(-1, "invalid log level '"+arg+"'");
                return false;
            }
            break;
        case Layout:
            st = None;
            if(arg == "basic")
                layout = nullptr;
            if(arg == "simple")
                layout = new log4cxx::SimpleLayout();
            else if(arg == "pattern")
                st = Pattern;
            else if(arg == "xml")
                layout = new log4cxx::xml::XMLLayout();
            else if(arg == "html")
                layout = new log4cxx::HTMLLayout();
            else
            {
                ec.add(-1, "invalid layout '"+arg+"'");
                return false;
            }
            break;
        case Pattern:
            st = None;
            layout = new log4cxx::PatternLayout(arg);
            break;
        case Appender:
            st = None;
            if(arg == "file")
                st = Filename;
            else if(arg == "console")
                st = Target;
            else
            {
                ec.add(-1, "invalid appender '"+arg+"'");
                return false;
            }
            break;
        case Filename:
            st = None;
            appender = new log4cxx::FileAppender(new log4cxx::SimpleLayout, arg, true);
            break;
        case Target:
            st = None;
            appender = new log4cxx::ConsoleAppender(new log4cxx::SimpleLayout, arg);
        }
	}

    if(st != None)
    {
        ec.add(-1, "invalid command line arguments");
        return false;
    }

    if(basic_config)
    {
        if(appender && layout)
            appender->setLayout(layout);
        else if(layout)
            appender = new log4cxx::ConsoleAppender(layout);

        if(appender)
            log4cxx::BasicConfigurator::configure(appender);
        else
            log4cxx::BasicConfigurator::configure();
    }

    if(level)
        log4cxx::Logger::getRootLogger()->setLevel(level);

#else // SAS_LOG4CXX_ENABLED
    (void)ec;
	enum State
	{
		None,
		File,
		StdOut,
		StdErr,
		MinPrio
	} st(None), config_type(None);
	std::string filename;
	AbstractLogger::Priority min_prio(AbstractLogger::Priority::Info);
	for(int i = 0; i < argc ; ++i)
	{
		switch(st)
		{
		case None:
			if(st == None)
			{
				if(std::string(argv[i]) == "-log-stdout")
					config_type = StdOut;
				else if(std::string(argv[i]) == "-log-stderr")
					config_type = StdErr;
				else if(std::string(argv[i]) == "-log-file")
					st = File;
				else if(std::string(argv[i]) == "-log-min-prio")
					st = MinPrio;
			}
			break;
		case StdOut:
		case StdErr:
			break;
		case File:
			filename = argv[i];
			st = None;
			config_type = File;
			break;
		case MinPrio:
			if(std::string(argv[i]) == "trace")
				min_prio = AbstractLogger::Priority::Trace;
			else if(std::string(argv[i]) == "debug")
				min_prio = AbstractLogger::Priority::Debug;
			else if(std::string(argv[i]) == "info")
				min_prio = AbstractLogger::Priority::Info;
			else if(std::string(argv[i]) == "warn")
				min_prio = AbstractLogger::Priority::Warn;
			else if(std::string(argv[i]) == "error")
				min_prio = AbstractLogger::Priority::Error;
			else if(std::string(argv[i]) == "fatal")
		min_prio = AbstractLogger::Priority::Fatal;
			st = None;
		}
	}

	switch(config_type)
	{
	case None:
	case MinPrio:
		break;
	case StdOut:
		setLogging(new StreamLogging<std::ostream>(std::cout, min_prio));
		break;
	case StdErr:
		setLogging(new StreamLogging<std::ostream>(std::cerr, min_prio));
		break;
	case File:
	{
		struct FileLogging : public StreamLogging<std::ofstream>
		{
		public:
			inline FileLogging(AbstractLogger::Priority min_prio) : StreamLogging<std::ofstream>(fs, min_prio)
			{ }
			std::ofstream fs;
		};
		auto logging = new FileLogging(min_prio);
		logging->fs.open(filename, std::fstream::in | std::fstream::out | std::fstream::app);
		if(logging->fs.fail())
		{
			ec.add(SAS_BASICS__ERROR__LOGGING__CANNOT_OPEN_LOGFILE, "could not open log file.");
			return false;
		}
		setLogging(logging);
	}
	}

#endif // SAS_LOG4CXX_ENABLED

	return true;
}

extern SAS_BASICS__FUNCTION void writeUsage(std::ostream & os)
{
	os << "Logging Options:" << std::endl;
#ifdef SAS_LOG4CXX_ENABLED
	os << "\t-log4cxx-xml-config <xml_file>" << std::endl;
	os << "\t-log4cxx-property-config <property_file>" << std::endl;
	os << "\t-log4cxx-basic-config" << std::endl;
    os << "\t-log4cxx-level {trace|debug|info|warn|error|fatal|all|off}" << std::endl;
    os << "\t-log4cxx-appender {{console <target>}|{file <filename>}}" << std::endl;
    os << "\t-log4cxx-layout {{pattern <pattern>}|simple|basic|xml|html}" << std::endl;
#else
	os << "\t-log-stdout" << std::endl;
	os << "\t-log-stderr" << std::endl;
	os << "\t-log-file <log file>" << std::endl;
	os << "\t-log-min_prio {trace|debug|info|warn|error|fatal}" << std::endl;
#endif // SAS_LOG4CXX_ENABLED
}

}}
