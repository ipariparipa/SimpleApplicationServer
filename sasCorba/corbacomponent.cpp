/*
    This file is part of sasCorba.

    sasCorba is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasCorba is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasCorba.  If not, see <http://www.gnu.org/licenses/>
 */
#include <sasCore/component.h>
#include <sasCore/application.h>
#include <sasCore/interfacemanager.h>
#include <sasCore/objectregistry.h>
#include <sasCore/configreader.h>
#include <sasCore/logging.h>

#include "corbainterface.h"
#include "corbaconnector.h"

#include <omniORB4/CORBA.h>

#include <numeric>

namespace SAS {

class CorbaComponent : public Component
{
public:
	CorbaComponent() : SAS::Component(), app(nullptr), logger(Logging::getLogger("SAS.CorbaComponent"))
	{ }

	virtual bool init(Application * app, ErrorCollector & ec) override
	{
		SAS_LOG_NDC();

		this->app = app;

		try
		{

			std::vector<std::string> ops;
			if (!app->configreader()->getStringListEntry("SAS/CORBA/OPTIONS", ops, ec))
			{
				SAS_LOG_INFO(logger, "no options are set in configureation, using commanf line arguments");
				int argc = app->argc();
				char ** argv = app->argv();
				orb = CORBA::ORB_init(argc, argv);
			}
			else
			{
				SAS_LOG_VAR_NAME(logger, "options", Logging::toString(ops));
				std::vector<const char *> ops_buff(ops.size());
				for (size_t i(0), l(ops.size()); i < l; ++i)
					ops_buff[i] = ops[i].c_str();
				int argc = ops_buff.size();
				char ** argv = (char **)ops_buff.data();
				orb = CORBA::ORB_init(argc, argv);
			}
		}
		catch (CORBA::COMM_FAILURE &)
		{
			auto err = ec.add(-1, "Caught system exception COMM_FAILURE.");
			SAS_LOG_ERROR(logger, err);
			return false;
		}
		catch (CORBA::SystemException &)
		{
			auto err = ec.add(-1, "Caught a CORBA::SystemException.");
			SAS_LOG_ERROR(logger, err);
			return false;
		}
		catch (...)
		{
			auto err = ec.add(-1, "Caught a sas::error_handling::invoker_error_exception while using the naming service.");
			SAS_LOG_ERROR(logger, err);
			return false;
		}

		if(CORBA::is_nil(orb))
		{
			auto err = ec.add(-1, "could not initialize ORB");
			SAS_LOG_ERROR(logger, err);
			return false;
		}

		auto im = app->interfaceManager();
		if(im)
		{
			std::vector<std::string> interface_names;
			if(app->configreader()->getStringListEntry("SAS/CORBA/INTERFACES", interface_names, ec))
			{
				bool has_error(false);
				std::vector<SAS::Interface*> interfaces(interface_names.size());
				for(size_t i = 0, l = interface_names.size(); i < l; ++i)
				{
					auto intf = new CorbaInterface(interface_names[i], app);
					if(!intf->init(orb, std::string("SAS/CORBA/") + interface_names[i], ec))
						has_error = true;
					else
						interfaces[i] = intf;
				}
				if(has_error)
					return false;

				if(!im->registerInterfaces(interfaces, ec))
					return false;
			}
		}

		std::vector<std::string> connector_names;
		if(app->configreader()->getStringListEntry("SAS/CORBA/CONNECTORS", connector_names, ec))
		{
			bool has_error(false);
			std::vector<Object*> connectors(connector_names.size());
			for(size_t i = 0, l = connector_names.size(); i < l; ++i)
			{
				auto conn = new CorbaConnector(connector_names[i], app);
				if(!conn->init(orb, std::string("SAS/CORBA/") + connector_names[i], ec))
					has_error = true;
				else
					connectors[i] = conn;
			}
			if(has_error)
				return false;

			if(!app->objectRegistry()->registerObjects(connectors, ec))
				return false;
		}

		return true;
	}

	virtual std::string name() const override
	{
		return "SAS Corba";
	}

	virtual std::string version() const override
	{
		return "0.1";
	}

private:
	Application * app;
	CORBA::ORB_var orb;
	Logging::LoggerPtr logger;
};

}


extern "C" SAS_CORBA__FUNCTION SAS::Component * __sas_attach_component()
{
	return new SAS::CorbaComponent;
}

extern "C" SAS_CORBA__FUNCTION void __sas_detach_component(SAS::Component * c)
{
	delete c;
}
