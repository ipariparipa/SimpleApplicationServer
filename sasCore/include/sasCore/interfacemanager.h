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

#ifndef INCLUDE_SASCORE_INTERFACEMANAGER_H_
#define INCLUDE_SASCORE_INTERFACEMANAGER_H_

#include "defines.h"
#include "logging.h"

#include <vector>

namespace SAS {

class ErrorCollector;
class Interface;
class Watchdog;
class Application;

struct InterfaceManager_priv;

class SAS_CORE__CLASS InterfaceManager
{
	SAS_COPY_PROTECTOR(InterfaceManager)
public:
    InterfaceManager(Application * app);
	virtual ~InterfaceManager();

	bool start(ErrorCollector & ec);
	void terminate();
	bool stop(ErrorCollector & ec);
	bool stopOrTerminate(long waitUntil, ErrorCollector & ec);
	bool wait(long waitUntil, ErrorCollector & ec);
	void wait();

	bool registerInterface(Interface * interface, ErrorCollector & ec);
	bool registerInterfaces(const std::vector<Interface *> & interfaces, ErrorCollector & ec);

	virtual Logging::LoggerPtr logger() = 0;

	virtual Watchdog * watchdog() = 0;
private:
	InterfaceManager_priv * priv;
};

}

#endif /* INCLUDE_SASCORE_INTERFACEMANAGER_H_ */
