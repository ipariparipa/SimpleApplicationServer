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

#ifndef sasMQTT__mqttclient_h
#define sasMQTT__mqttclient_h

#include "config.h"
#include <sasCore/defines.h>
#include <sasCore/logging.h>

#include <memory>
#include <vector>

namespace SAS {

	class ErrorCollector;
	struct MQTTConnectionOptions;

	struct MQTTClient_priv;

	class MQTTClient
	{
		SAS_COPY_PROTECTOR(MQTTClient);
	public:
		MQTTClient(const std::string & name);
		MQTTClient(const Logging::LoggerPtr & logger);
		~MQTTClient();

		bool init(const MQTTConnectionOptions & options, ErrorCollector & ec);
		void deinit();

		bool publish(const std::string & topic, const std::vector<char> & payload, long qus, long timeout, ErrorCollector & ec);
		bool receive(const std::vector<std::string> & subscribe, long qus, std::string & topic, std::vector<char> & payload, long timeout, long count, ErrorCollector & ec);
		bool receive(const std::vector<std::string> & subscribe, long qus, std::string & topic, std::vector<char> & payload, long timeout, ErrorCollector & ec);

		bool connect(ErrorCollector & ec);
		bool disconnect(long timeout, ErrorCollector & ec);


	private:
		std::unique_ptr<MQTTClient_priv> priv;
	};

}

#endif // sasMQTT__mqttclient_h
