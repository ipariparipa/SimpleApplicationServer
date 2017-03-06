/*
    This file is part of sasBypass.

    sasBypass is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasBypass is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasBypass.  If not, see <http://www.gnu.org/licenses/>
 */

#include "config.h"
#include <sasCore/component.h>
#include <sasCore/configreader.h>
#include <sasCore/logging.h>
#include <sasCore/application.h>
#include <sasCore/objectregistry.h>

#include "loopbackconnector.h"
#include "bypassmodule.h"

namespace SAS {

class BP_Component : public Component
{
	SAS_COPY_PROTECTOR(BP_Component)
public:
	BP_Component() : Component()
	{ }

	virtual std::string name() const
	{
		return "SAS Bypass";
	}

	virtual std::string version() const
	{
		return "1.0";
	}

	virtual std::vector<Module*> modules() const final
	{
		return std::vector<Module*>();
	}

	virtual bool init(Application * app, ErrorCollector & ec) final
	{
		SAS_LOG_NDC();
		Logging::LoggerPtr logger = Logging::getLogger("SAS.BP_Component");

		{
			std::vector<std::string> loopback_names;
			if (app->configReader()->getStringListEntry("SAS/BYPASS/LOOPBACK_CONNECTORS", loopback_names, ec) && loopback_names.size())
			{
				std::vector<Object*> objs(loopback_names.size());
				for (size_t i(0), l(loopback_names.size()); i < l; ++i)
					objs[i] = new LoopbackConnector(app, loopback_names[i]);

				if (!app->objectRegistry()->registerObjects(objs, ec))
					return false;
			}
			else
			{
				SAS_LOG_INFO(logger, "no loopback connectors are set");
			}
		}

		{
			std::vector<std::string> bypassmodule_names;
			if (app->configReader()->getStringListEntry("SAS/BYPASS/MODULES", bypassmodule_names, ec) && bypassmodule_names.size())
			{
				std::vector<Object*> objs(bypassmodule_names.size());
				bool has_error(false);
				for (size_t i(0), l(bypassmodule_names.size()); i < l; ++i)
				{
					auto mod = new BypassModule(bypassmodule_names[i]);
					std::string config_path = "SAS/BYPASS/" + bypassmodule_names[i];
					if (!mod->init(config_path, app, ec))
						has_error = true;
					else
						objs[i] = mod;
				}
				
				if (has_error)
					return false;

				if (!app->objectRegistry()->registerObjects(objs, ec))
					return false;
			}
			else
			{
				SAS_LOG_INFO(logger, "no bypass modules are set");
			}
		}

		return true;
	}

};

}

extern "C" SAS_BYPASS__FUNCTION SAS::Component * __sas_attach_component()
{
	return new SAS::BP_Component;
}

extern "C" SAS_BYPASS__FUNCTION void __sas_detach_component(SAS::Component * c)
{
	delete c;
}
