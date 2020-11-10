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

#ifndef sasMQTT__MQTTCONNECTORFACTRORY_H
#define sasMQTT__MQTTCONNECTORFACTRORY_H

#include <sasCore/connectorfactory.h>
#include <memory>

namespace SAS {

    class Application;

    class MQTTConnectorFactory : public ConnectorFactory
    {
        SAS_COPY_PROTECTOR(MQTTConnectorFactory)

        struct Priv;
        std::unique_ptr<Priv> p;
    public:
        MQTTConnectorFactory(const std::string & name, Application * app);

        virtual ~MQTTConnectorFactory() override;

        std::string name() const final override;

        bool init(const std::string & path, ErrorCollector & ec);

        Connector * make(const std::string & name, const std::string & connectionString, ErrorCollector & ec) final override;
    };

}

#endif // sasMQTT__MQTTCONNECTORFACTRORY_H
