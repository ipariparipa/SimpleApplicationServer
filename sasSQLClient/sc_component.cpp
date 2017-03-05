/*
    This file is part of sasSQLClient.

    sasSQLClient is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasSQLClient is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasSQLClient.  If not, see <http://www.gnu.org/licenses/>
 */

#include <sasCore/component.h>
#include <sasCore/logging.h>
#include <sasCore/application.h>
#include <sasCore/configreader.h>
#include <sasCore/objectregistry.h>

#include "sc_module.h"

namespace SAS { namespace SQLClient
{

class SC_Component : public Component
{
public:
	SC_Component() : Component(), logger(Logging::getLogger("SAS.SQLClient.Component"))
	{ }

	virtual ~SC_Component()
	{ }

	virtual bool init(Application * app, ErrorCollector & ec) final
	{
		SAS_LOG_NDC();

		std::vector<std::string> module_names;
		if(app->configreader()->getStringListEntry("SAS/SQL_CLIENT/MODULES", module_names, ec))
		{
			std::vector<Object*> objs(module_names.size());
			_modules.resize(module_names.size());
			bool has_error(false);
			size_t i(0);
			for(auto & name : module_names)
			{
				auto m = new SC_Module(name);
				if(!m->init(app, ec))
				{
					delete m;
					has_error = true;
				}
				else
					objs[i] = _modules[i]= m;
				++i;
			}
			if(has_error)
				return false;
			if(!app->objectRegistry()->registerObjects(objs, ec))
				return false;
		}
		else
			SAS_LOG_INFO(logger, "no module names are set");

		return true;
	}

	virtual std::string name() const
	{
		return "SAS SQLClient";
	}

	virtual std::string version() const
	{
		return "0.1";
	}

	virtual std::vector<Module*> modules() const
	{
		return _modules;
	}

private:
	Logging::LoggerPtr logger;
	std::vector<Module*> _modules;
};

}}

extern "C" SAS::Component * __sas_attach_component()
{
	return new SAS::SQLClient::SC_Component;
}

extern "C" void __sas_detach_component(SAS::Component * c)
{
	delete c;
}

