/*
    This file is part of sas.

    sas is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sas is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sas.  If not, see <http://www.gnu.org/licenses/>
 */

#include <log4cxx/basicconfigurator.h>
#include <sasBasics/logging.h>
#include <sasCore/errorcollector.h>
#include <sasBasics/streamerrorcollector.h>
#include "sasserver.h"

#include <sasCore/module.h>
#include <sasCore/session.h>
#include <sasCore/invoker.h>

#include <thread>
#include <iostream>
#include <sstream>

int main(int argc, char * argv[])
{
	for(int i(0); i < argc; ++i)
	{
		if(std::string(argv[i]) == "--help")
			SAS::Logging::writeUsage(std::cout);
	}

	SAS::StreamErrorCollector<> ec(std::cerr);

	SAS::Logging::init(argc, argv, ec);

	SAS_LOG_NDC();

	SAS_ROOT_LOG_INFO("starting");

	SAS::SASServer server;

	SAS_ROOT_LOG_TRACE("initializing SAS");
	if(!server.init(ec))
	{
		SAS_ROOT_LOG_ERROR("could not initialize SAS");
		return 1;
	}

	SAS_ROOT_LOG_TRACE("starting SASServer");
	if(!server.start(ec))
	{
		SAS_ROOT_LOG_ERROR("could not start SAS");
		return 1;
	}

	SAS_ROOT_LOG_TRACE("start watchdog");
	server.run();

//	std::this_thread::sleep_for(std::chrono::seconds(10000));

	return 0;
}
