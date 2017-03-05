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

#ifndef INCLUDE_SASCORE_APPLICATION_H_
#define INCLUDE_SASCORE_APPLICATION_H_

#include "defines.h"
#include "errorcollector.h"
#include "logging.h"

#include <vector>

namespace SAS
{

class ConfigReader;
class InterfaceManager;
class ObjectRegistry;

struct Application_priv;
class Application
{
SAS_COPY_PROTECTOR(Application);
public:
	Application();
	virtual ~Application();

	ObjectRegistry * objectRegistry() const;

	virtual inline std::string version() const { return std::string(); }

	virtual bool init(ErrorCollector & ec);
	virtual void deinit();

	virtual Logging::LoggerPtr logger();

	virtual inline InterfaceManager * interfaceManager() { return nullptr; };
	virtual ConfigReader * configreader() = 0;
private:
	Application_priv * priv;
};

}

#endif /* INCLUDE_SASCORE_APPLICATION_H_ */
