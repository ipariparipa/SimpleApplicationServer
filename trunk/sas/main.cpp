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

#include <sasBasics/logging.h>
#include <sasCore/errorcollector.h>
#include <sasBasics/streamerrorcollector.h>
#include "sasserver.h"
#include "version.h"

#include <sasCore/module.h>
#include <sasCore/session.h>
#include <sasCore/invoker.h>

#include <thread>
#include <iostream>
#include <sstream>
#include <fstream>

void writeVersion(std::ostream & os)
{
	os << "Version: " << SAS_SERVER_VERSION << std::endl;
}

void writeHelp(std::ostream & os)
{
	writeVersion(os);
	os << std::endl;
	os << "Error Handling Options:" << std::endl;
	os << "\t-ec-stdout" << std::endl;
	os << "\t-ec-stderr" << std::endl;
	os << std::endl;
	SAS::Logging::writeUsage(os);
}

int main(int argc, char * argv[])
{
	std::ostream * err_os = &std::cerr;
	enum class CLA_Status
	{
		None,
		ECFile
	} cla_status = CLA_Status::None;
	bool has_error(false);
	std::ofstream ec_file_os;
	for(int i(1); i < argc; ++i)
	{
		std::string _argv = argv[i];
		switch (cla_status)
		{
		case CLA_Status::None:
			if (_argv == "--help")
			{
				writeHelp(std::cout);
				return 0;
			}
			else if (_argv == "--version")
			{
				writeVersion(std::cout);
				return 0;
			}
			else if (_argv == "-ec-stdout")
				err_os = &std::cout;
			else if (_argv == "-ec-stderr")
				err_os = &std::cerr;
			else if (_argv == "-ec-file")
				cla_status = CLA_Status::ECFile;
			break;
		case CLA_Status::ECFile:
			cla_status = CLA_Status::None;
			ec_file_os.open(_argv, std::ios::app);
			if (ec_file_os.fail())
			{
				std::cerr << "could not open file '" << _argv << "' to collect errors" << std::endl;
				return 1;
			}
			err_os = &ec_file_os;
			break;
		}
	}

	if (has_error)
		return 1;

	SAS::StreamErrorCollector<> ec(*err_os);

	SAS::Logging::init(argc, argv, ec);

	SAS_LOG_NDC();

	SAS_ROOT_LOG_INFO("starting");

	SAS::SASServer server(argc, argv);

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

	return 0;
}
