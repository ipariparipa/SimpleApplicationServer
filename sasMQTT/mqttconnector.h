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

#ifndef sasMQTT__mqttconnector_h
#define sasMQTT__mqttconnector_h

#include "include/sasMQTT/config.h"

#include <sasCore/connector.h>

#include <memory>

namespace SAS {

	class Application;

	class MQTTConnector_internal;

	struct MQTTConnector_priv;

	class MQTTConnector : public Connector
	{
		friend class MQTTConnector_internal;
	public:
		MQTTConnector(const std::string & name, Application * app);
		virtual ~MQTTConnector();

		virtual std::string name() const final;

		virtual bool init(const std::string & path, ErrorCollector & ec) final;

		virtual bool connect(ErrorCollector & ec) final;

		virtual Connection * createConnection(const std::string & module_name, const std::string & invoker_name, ErrorCollector & ec) final;

		virtual bool getModuleInfo(const std::string & moduleName, std::string & description, std::string & version, ErrorCollector & ec) final;

	private:

		std::unique_ptr<MQTTConnector_priv> priv;
	};

}

#endif // sasMQTT__mqttconnector_h
