/*
    This file is part of sasCorba.

    sasCorba is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasCorba is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasCorba.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef CORBAINTERFACE_H_
#define CORBAINTERFACE_H_

#include <sasCore/interface.h>
#include <sasCore/logging.h>

#include <omniORB4/CORBA.h>

#include <string>

namespace SAS {

class CorbaServer;

class Application;
class ErrorCollector;

class CorbaInterface : public Interface
{
public:
	CorbaInterface(const std::string & name, Application * app);

	virtual std::string name() const final;
	virtual Status run(ErrorCollector & ec) final;

	bool init(const CORBA::ORB_var & orb, const std::string & config_path, ErrorCollector & ec);

	Logging::LoggerPtr logger() const;

private:
	Application * _app;
	CorbaServer * _runner;
	Logging::LoggerPtr _logger;
	std::string _name;
};

}

#endif /* CORBAINTERFACE_H_ */
