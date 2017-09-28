/*
This file is part of sasMQTT.

sasMQTT is free software: you can redistribute it and/or modify
it under the terms of the Lesser GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

sasMQTT is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with sasMQTT.  If not, see <http://www.gnu.org/licenses/>
*/

#ifndef sasMQTT__mqttconnectionoptions_h
#define sasMQTT__mqttconnectionoptions_h

#include "config.h"
#include <string>

namespace SAS {

	class ConfigReader;
	class ErrorCollector;

	struct MQTTConnectionOptions
	{
		std::string clientId;
		std::string serverUri;
		long keepalive = 100; //secs
		std::string username;
		std::string password;
		bool cleanSession = true;
		long connectTimeout = 30; //secs
		long retryInterval = 20; //secs

		long receive_timeout = 1000; //msecs
		long publish_timeout = 1000; //msecs

		bool build(const std::string & config_path, ConfigReader * cr, ErrorCollector & ec);
	};

}

#endif // sasMQTT__mqttconnectionoptions_h
