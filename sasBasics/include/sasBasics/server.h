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

#ifndef INCLUDE_SASBASICS_SERVER_H_
#define INCLUDE_SASBASICS_SERVER_H_

#include "config.h"

#include <sasCore/application.h>
#include <sasCore/watchdog.h>
#include <sasCore/interfacemanager.h>

namespace SAS
{

struct Server_priv;

class SAS_BASICS__CLASS Server : public Application, public InterfaceManager, public Watchdog
{
	SAS_COPY_PROTECTOR(Server)
public:
	Server(int argc, char ** argv);
	virtual ~Server();

	virtual inline Watchdog * watchdog() override { return this; }
	virtual inline InterfaceManager * interfaceManager() override { return this; }

	virtual Logging::LoggerPtr logger() override;

	virtual void addInterfaceEvent(Interface::Status status, Interface * intf, const std::string & message) override;

	virtual void run();
	virtual void shutdown();

private:
	Server_priv * priv;
};

}

#endif /* INCLUDE_SASBASICS_SERVER_H_ */
