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

#include "mqttconnectorfactrory.h"

#include <sasCore/logging.h>
#include "mqttconnector.h"

namespace SAS {

    struct MQTTConnectorFactory::Priv
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
    };

    MQTTConnectorFactory::MQTTConnectorFactory(const std::string & name, Application * app) :
        ConnectorFactory(),
        p(new Priv(name, app))
    { }

    MQTTConnectorFactory::~MQTTConnectorFactory() = default;

    std::string MQTTConnectorFactory::name() const //final override
    {
        return p->name;
    }

    bool MQTTConnectorFactory::init(const std::string & path, ErrorCollector & ec)
    {
        SAS_LOG_NDC();

        (void)ec;
        p->configPath = path;
        return true;
    }

    Connector * MQTTConnectorFactory::make(const std::string & name, const std::string & connectionString, ErrorCollector & ec) //final override
    {
        SAS_LOG_NDC();

        auto ret = new MQTTConnector(name.length() ? name : p->name, p->app);
        if(!ret->init(connectionString, p->configPath, ec))
        {
            delete ret;
            return nullptr;
        }

        return ret;
    }

}
