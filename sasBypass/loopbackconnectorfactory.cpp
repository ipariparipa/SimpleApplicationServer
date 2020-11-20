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

#include "loopbackconnectorfactory.h"
#include <sasCore/logging.h>
#include "loopbackconnector.h"

namespace SAS {

    struct LoopbackConnectorFactory::Priv
    {
        Priv(const std::string & name, Application * app) :
            app(app),
            name(name),
            logger(Logging::getLogger("LoopbackConnectorFactory("+name+")"))
        { }

        Application * app;
        std::string name;
        Logging::LoggerPtr logger;
    };

    LoopbackConnectorFactory::LoopbackConnectorFactory(const std::string & name, Application * app) :
        ConnectorFactory(),
        p(new Priv(name, app))
    { }

    LoopbackConnectorFactory::~LoopbackConnectorFactory() = default;

    std::string LoopbackConnectorFactory::name() const //final override
    {
        return p->name;
    }

    Connector * LoopbackConnectorFactory::make(const std::string & name, const std::string & connectionString, ErrorCollector & ec) //final override
    {
        (void)connectionString;
        (void)ec;
        SAS_LOG_NDC();

        return new LoopbackConnector(p->app, name.length() ? name : p->name);
    }

}
