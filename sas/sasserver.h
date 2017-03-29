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

#ifndef SASSERVER_H_
#define SASSERVER_H_

#include <sasCore/application.h>
#include <sasCore/watchdog.h>
#include <sasCore/interfacemanager.h>

namespace SAS
{

struct SASServer_priv;

class SASServer : public Application, public InterfaceManager, public Watchdog
{

public:
	SASServer(int argc, char ** argv);
	virtual ~SASServer();

	virtual std::string version() const final;

	virtual ConfigReader * configReader() final;

	virtual inline Watchdog * watchdog() final { return this; }
	virtual inline InterfaceManager * interfaceManager() final { return this; }

	virtual Logging::LoggerPtr logger() final;

	virtual void addInterfaceEvent(Interface::Status status, Interface * intf, const std::string & message) final;
	virtual void run() final;

private:
	SASServer_priv * priv;
};

}

#endif /* SASSERVER_H_ */