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

	struct MQTTConnectionOptions
	{
		std::string clientId;
		std::string host;
		std::string port;
		long keepalive = 0; //secs
		std::string username;
		std::string password;

		long receive_timeout = 1000;
		long publish_timeout = 10000;
	};

}

#endif // sasMQTT__mqttconnectionoptions_h
