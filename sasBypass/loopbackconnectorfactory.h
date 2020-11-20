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

#ifndef sasBypass__LOOPBACKCONNECTORFACTORY_H
#define sasBypass__LOOPBACKCONNECTORFACTORY_H

#include <sasCore/connectorfactory.h>
#include <memory>

namespace SAS {

    class Application;

    class LoopbackConnectorFactory : public ConnectorFactory
    {
        SAS_COPY_PROTECTOR(LoopbackConnectorFactory)

        struct Priv;
        std::unique_ptr<Priv> p;
    public:
        LoopbackConnectorFactory(const std::string & name, Application * app);

        virtual ~LoopbackConnectorFactory() override;

        std::string name() const final override;

        Connector * make(const std::string & name, const std::string & connectionString, ErrorCollector & ec) final override;
    };

}

#endif // sasBypass__LOOPBACKCONNECTORFACTORY_H
