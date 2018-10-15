/*
This file is part of sasHTTP.

sasHTTP is free software: you can redistribute it and/or modify
it under the terms of the Lesser GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

sasHTTP is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with sasHTTP.  If not, see <http://www.gnu.org/licenses/>
*/

#include <sasCore/component.h>
#include <sasCore/application.h>
#include <sasCore/interfacemanager.h>
#include <sasCore/objectregistry.h>
#include <sasCore/configreader.h>
#include <sasCore/logging.h>
#include <sasCore/errorcodes.h>

#include "httpinterface.h"
#include "httpconnector.h"

#include <numeric>

#include <neon/ne_session.h>

namespace SAS {

	class HTTPComponent : public Component
	{
	public:
		HTTPComponent() : Component(), app(nullptr), logger(Logging::getLogger("SAS.HTTPComponent"))
		{ }

		virtual bool init(Application * app, ErrorCollector & ec) override
		{
			SAS_LOG_NDC();

			this->app = app;

			std::vector<std::string> interface_names;
			if (app->configReader()->getStringListEntry("SAS/HTTP/INTERFACES", interface_names, interface_names, ec))
			{
				if (interface_names.size())
				{
					auto im = app->interfaceManager();
					if (!im)
					{
						auto err = ec.add(-1, "interface manager is not existing, could not register HTTP interface");
						SAS_LOG_ERROR(logger, err);
						return false;
					}

					bool has_error(false);
					std::vector<Interface*> interfaces(interface_names.size());
					for (size_t i = 0, l = interface_names.size(); i < l; ++i)
					{
						auto intf = new HTTPInterface(interface_names[i], app);
						if (!intf->init(std::string("SAS/HTTP/") + interface_names[i], ec))
							has_error = true;
						else
							interfaces[i] = intf;
					}
					if (has_error)
						return false;

					if (!im->registerInterfaces(interfaces, ec))
						return false;
				}
				else
					SAS_LOG_INFO(logger, "no HTTP interface is defined.");
			}

			std::vector<std::string> connector_names;
			if (app->configReader()->getStringListEntry("SAS/HTTP/CONNECTORS", connector_names, connector_names, ec))
			{
				if (connector_names.size())
				{
					ne_sock_init();

					bool has_error(false);
					std::vector<Object*> connectors(connector_names.size());
					for (size_t i = 0, l = connector_names.size(); i < l; ++i)
					{
						auto conn = new HTTPConnector(connector_names[i], app);
						if (!conn->init(std::string("SAS/HTTP/") + connector_names[i], ec))
							has_error = true;
						else
							connectors[i] = conn;
					}
					if (has_error)
						return false;

					if (!app->objectRegistry()->registerObjects(connectors, ec))
						return false;
				}
				else
					SAS_LOG_INFO(logger, "no HTTP connector is defined.");
			}

			return true;
		}

		virtual std::string name() const override
		{
			return "SAS HTTP";
		}

		virtual std::string version() const override
		{
			return "0.1";
		}

	private:
		Application * app;
		Logging::LoggerPtr logger;
	};

}


extern "C" SAS_HTTP__FUNCTION SAS::Component * __sas_attach_component()
{
	return new SAS::HTTPComponent;
}

extern "C" SAS_HTTP__FUNCTION void __sas_detach_component(SAS::Component * c)
{
	delete c;
}
