/*
    This file is part of sasOracle.

    sasOracle is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasOracle is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasOracle.  If not, see <http://www.gnu.org/licenses/>
 */

#include "config.h"

#include <sasCore/component.h>
#include <sasCore/application.h>
#include <sasCore/objectregistry.h>
#include <sasCore/configreader.h>
#include <sasCore/logging.h>

#include <sasSQL/errorcodes.h>

#include "oraconnector.h"
#include "oratools.h"

#include SAS_ORACLE__DPI_H

namespace SAS {

class OraComponent : public Component
{
public:
	OraComponent() : SAS::Component(), app(nullptr)
	{ }

	virtual bool init(Application * app, ErrorCollector & ec) override
	{
		SAS_LOG_NDC();

		Logging::LoggerPtr logger = Logging::getLogger("SAS.OraComponent");

		this->app = app;

		dpiContext * ctx = NULL;
		dpiErrorInfo ei;
		if (dpiContext_create(DPI_MAJOR_VERSION, DPI_MINOR_VERSION, &ctx, &ei) != DPI_SUCCESS)
		{
			auto err = ec.add(SAS_SQL__ERROR__CANNOT_INIT_CONNECTOR_LIB, "could not initialize Oracle DPI: " + OraTools::toString(ei));
			SAS_LOG_ERROR(logger, err);
			return false;
		}

		std::vector<std::string> connector_names;
		if(app->configReader()->getStringListEntry("SAS/ORACLE/CONNECTORS", connector_names, ec))
		{
			bool has_error(false);
			std::vector<Object*> connectors(connector_names.size());
			for(size_t i = 0, l = connector_names.size(); i < l; ++i)
			{
				auto conn = new OraConnector(ctx, connector_names[i], app);
				if(!conn->init(std::string("SAS/ORACLE/") + connector_names[i], ec))
					has_error = true;
				else
					connectors[i] = conn;
			}
			if(has_error)
				return false;

			return app->objectRegistry()->registerObjects(connectors, ec);
		}
		else
			SAS_LOG_INFO(logger, "no connectros are defined");

		return true;
	}

	virtual std::string name() const override
	{
		return "SAS Oracle";
	}

	virtual std::string version() const override
	{
		return "0.1";
	}

private:
	Application * app;
};

}

extern "C" SAS_ORACLE__FUNCTION SAS::Component * __sas_attach_component()
{
	return new SAS::OraComponent;
}

extern "C" SAS_ORACLE__FUNCTION void __sas_detach_component(SAS::Component * c)
{
	delete c;
}
