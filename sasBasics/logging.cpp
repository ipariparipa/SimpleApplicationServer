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

#include <sasCore/errorcollector.h>

#include <log4cxx/basicconfigurator.h>
#include <log4cxx/xml/domconfigurator.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/consoleappender.h>

namespace SAS { namespace Logging {

log4cxx::LoggerPtr _rootLogger;

bool init(int argc, char *argv[], ErrorCollector & ec)
{
#ifdef SAS_LOG4CXX_ENABLED
	enum State
	{
		None,
		XMLConfig,
		PropertyConfig,
		BaseConfig
	} st(None), config_type(None);
	for(int i = 0; i < argc ; ++i)
	{
		switch(st)
		{
		case None:
			if(config_type == None)
			{
				if(std::string(argv[i]) == "-log4cxx-xml-config")
					st = XMLConfig;
				else if(std::string(argv[i]) == "-log4cxx-property-config")
					st = PropertyConfig;
				else if(std::string(argv[i]) == "-log4cxx-basic-config")
					st = PropertyConfig;
			}
			break;
		case XMLConfig:
			log4cxx::xml::DOMConfigurator::configure(argv[i]);
			config_type = st;
			st = None;
			break;
		case PropertyConfig:
			log4cxx::PropertyConfigurator::configure(argv[i]);
			config_type = st;
			st = None;
			break;
		case BaseConfig:
			log4cxx::BasicConfigurator::configure(new log4cxx::ConsoleAppender());
			config_type = st;
			st = None;
			break;
		}
	}
	if(config_type == None)
		log4cxx::BasicConfigurator::configure();

#endif // SAS_LOG4CXX_ENABLED

	return true;
}

void writeUsage(std::ostream & os)
{
#ifdef SAS_LOG4CXX_ENABLED
	os << "Logging options:" << std::endl;
	os << "\t-log4cxx-xml-config" << std::endl;
	os << "\t-log4cxx-property-config" << std::endl;
	os << "\t-log4cxx-basic-config" << std::endl;
#endif // SAS_LOG4CXX_ENABLED
}

}}
