/*
    This file is part of sasMySQL.

    sasMySQL is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasMySQL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasMySQL.  If not, see <http://www.gnu.org/licenses/>
 */

#include "config.h"

#include <sasCore/component.h>
#include <sasCore/application.h>
#include <sasCore/objectregistry.h>
#include <sasCore/configreader.h>
#include <sasCore/logging.h>

#include "mysqlconnector.h"

#include <mysql/mysql.h>

namespace SAS {

class MySQLComponent : public Component
{
public:
	MySQLComponent() : SAS::Component(), app(nullptr)
	{ }

	virtual bool init(Application * app, ErrorCollector & ec) override
	{
		SAS_LOG_NDC();

		Logging::LoggerPtr logger = Logging::getLogger("SAS.MySQLComponent");

		this->app = app;

		std::vector<std::string> connector_names;
		if(app->configreader()->getStringListEntry("SAS/MYSQL/CONNECTORS", connector_names, ec))
		{
			bool has_error(false);
			std::vector<Object*> connectors(connector_names.size());
			for(size_t i = 0, l = connector_names.size(); i < l; ++i)
			{
				auto conn = new MySQLConnector(connector_names[i], app);
				if(!conn->init(std::string("SAS/MYSQL/") + connector_names[i], ec))
					has_error = true;
				else
					connectors[i] = conn;
			}
			if(has_error)
				return false;

			return app->objectRegistry()->registerObjects(connectors, ec);
		}
		SAS_LOG_INFO(logger, "no connectros are defined");

		if (mysql_library_init(app->argc(), app->argv(), nullptr))
		{
			auto err = ec.add(-1, "could not initialize MySQL library");
			SAS_LOG_ERROR(logger, err);
			return false;
		}

		return true;
	}

	virtual std::string name() const override
	{
		return "SAS MySQL";
	}

	virtual std::string version() const override
	{
		return "0.1";
	}

private:
	Application * app;
};

}

extern "C" SAS_MYSQL__FUNCTION SAS::Component * __sas_attach_component()
{
	return new SAS::MySQLComponent;
}

extern "C" SAS_MYSQL__FUNCTION void __sas_detach_component(SAS::Component * c)
{
	delete c;
}
