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

#include "corbaconnectorfactory.h"

#include <sasCore/logging.h>
#include "corbaconnector.h"

namespace SAS {

    struct CorbaConnectorFactory::Priv
    {
        Priv(const std::string & name, Application * app) :
            app(app),
            name(name),
            logger(Logging::getLogger("MQTTConnectorFactory("+name+")"))
        { }

        Application * app;
        std::string name;
        Logging::LoggerPtr logger;
        std::string configPath;
        CORBA::ORB_var orb;
    };

    CorbaConnectorFactory::CorbaConnectorFactory(const std::string & name, Application * app) :
        ConnectorFactory(),
        p(new Priv(name, app))
    { }

    CorbaConnectorFactory::~CorbaConnectorFactory() = default;

    std::string CorbaConnectorFactory::name() const //final override
    {
        return p->name;
    }

    bool CorbaConnectorFactory::init(const CORBA::ORB_var & orb, const std::string & path, ErrorCollector & ec)
    {
        SAS_LOG_NDC();

        (void)ec;
        p->configPath = path;
        p->orb = orb;
        return true;
    }

    Connector * CorbaConnectorFactory::make(const std::string & name, const std::string & connectionString, ErrorCollector & ec) //final override
    {
        SAS_LOG_NDC();

        auto ret = new CorbaConnector(name.length() ? name : p->name, p->app);
        if(!ret->init(p->orb, connectionString, p->configPath, ec))
        {
            delete ret;
            return nullptr;
        }

        return ret;
    }

}
