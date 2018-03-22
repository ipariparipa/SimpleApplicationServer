/*
    This file is part of sasODBC.

    sasODBC is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasODBC is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasODBC.  If not, see <http://www.gnu.org/licenses/>
 */

#include "config.h"

#include <sasCore/component.h>
#include <sasCore/application.h>
#include <sasCore/objectregistry.h>
#include <sasCore/configreader.h>
#include <sasCore/logging.h>

#include <sasSQL/errorcodes.h>

#include "odbcconnector.h"
#include "odbctools.h"

#include "include_odbc.h"

namespace SAS {

class ODBCComponent : public Component
{
public:
	ODBCComponent() : SAS::Component(), app(nullptr)
	{ }

	virtual ~ODBCComponent() { }

	virtual bool init(Application * app, ErrorCollector & ec) override
	{
		SAS_LOG_NDC();

		Logging::LoggerPtr logger = Logging::getLogger("SAS.ODBCComponent");

		this->app = app;

		SQLRETURN rc;

		std::vector<std::string> connector_names;
		if(app->configReader()->getStringListEntry("SAS/ODBC/CONNECTORS", connector_names, ec))
		{
			SQLHENV env = 0;
			SAS_LOG_TRACE(logger, "SQLAllocEnv");
			if (!SQL_SUCCEEDED(rc = SQLAllocEnv(&env)))
			{
				auto err = ec.add(SAS_SQL__ERROR__CANNOT_INIT_CONNECTOR_LIB, "could not allocate ODBC environment");
				SAS_LOG_ERROR(logger, err);
				return false;
			}

			assert(env);

			if (!SQL_SUCCEEDED(rc = SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_UINTEGER)))
			{
				auto err = ec.add(SAS_SQL__ERROR__CANNOT_INIT_CONNECTOR_LIB, "could not set ODBC version: " + ODBCTools::getError(env, SQL_NULL_HANDLE, SQL_NULL_HANDLE, rc));
				SAS_LOG_ERROR(logger, err);
				SAS_LOG_TRACE(logger, "SQLFreeEnv");
				if (!SQL_SUCCEEDED(rc = SQLFreeEnv(env)))
				{
					auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "unexpected error: " + ODBCTools::getError(env, SQL_NULL_HANDLE, SQL_NULL_HANDLE, rc));
					SAS_LOG_ERROR(logger, err);
				}
				return false;
			}

			std::vector<Object*> connectors(connector_names.size());
			for(size_t i = 0, l = connector_names.size(); i < l; ++i)
			{
				auto conn = new ODBCConnector(env, connector_names[i], app);
				if(conn->init(std::string("SAS/ODBC/") + connector_names[i], ec))
					connectors[i] = conn;
			}

			return app->objectRegistry()->registerObjects(connectors, ec);
		}
		else
			SAS_LOG_INFO(logger, "no connectros are defined");

		return true;
	}

	virtual std::string name() const override
	{
		return "SAS ODBC";
	}

	virtual std::string version() const override
	{
		return "0.1";
	}

private:
	Application * app;
};

}

extern "C" SAS_ODBC__FUNCTION SAS::Component * __sas_attach_component()
{
	return new SAS::ODBCComponent;
}

extern "C" SAS_ODBC__FUNCTION void __sas_detach_component(SAS::Component * c)
{
	delete c;
}
