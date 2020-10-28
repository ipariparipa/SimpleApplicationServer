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

#ifndef sasMQTT__mqttconnectionoptions_h
#define sasMQTT__mqttconnectionoptions_h

#include "config.h"
#include <string>
#include <chrono>
#include <memory>

namespace SAS {

	class ConfigReader;
	class ErrorCollector;

    class MQTTConnectionOptions
	{
        struct Priv;
        std::unique_ptr<Priv> p;
    public:
        MQTTConnectionOptions();
        ~MQTTConnectionOptions();

        MQTTConnectionOptions(const MQTTConnectionOptions & o);
        MQTTConnectionOptions & operator = (const MQTTConnectionOptions & o);

        const std::string & clientId() const;
        void setClientId(const std::string & v);

        const std::string & serverUri() const;
        void setServerUri(const std::string & v);

        std::chrono::seconds keepalive() const;
        void setKeepalive(std::chrono::seconds v) const;

        const std::string & username() const;
        void setUsername(const std::string & v) const;

        const std::string & password() const;
        void setPassword(const std::string & v) const;

        bool cleanSession() const;
        void setCleanSession(bool v) const;

        std::chrono::seconds connectTimeout() const;
        void setConnectTimeout(std::chrono::seconds v) const;

        std::chrono::seconds retryInterval() const;
        void setRetryInterval(std::chrono::seconds ) const;

        std::chrono::milliseconds receiveTimeout() const;
        void setReceiveTimeout(std::chrono::milliseconds v) const;

        std::chrono::milliseconds publishTimeout() const;
        void setPublishTimeout(std::chrono::milliseconds v) const;


		bool build(const std::string & config_path, ConfigReader * cr, ErrorCollector & ec);
        bool build(const std::string & connection_str, const std::string & config_path, ConfigReader * cr, ErrorCollector & ec);
    };

}

#endif // sasMQTT__mqttconnectionoptions_h
