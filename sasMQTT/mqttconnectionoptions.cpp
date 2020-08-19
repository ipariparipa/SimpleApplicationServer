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

#include "include/sasMQTT/mqttconnectionoptions.h"

#include <sasCore/errorcollector.h>
#include <sasCore/configreader.h>
#include <sasCore/tools.h>
#include <sasCore/logging.h>

namespace SAS {

    struct MQTTConnectionOptions::Priv
    {
        Priv() :
            keepalive(100),
            connectTimeout(30),
            retryInterval(20),
            receive_timeout(1000),
            publish_timeout(1000)
        { }

        std::string clientId;
        std::string serverUri;
        std::chrono::seconds keepalive;
        std::string username;
        std::string password;
        bool cleanSession = true;
        std::chrono::seconds connectTimeout;
        std::chrono::seconds retryInterval;

        std::chrono::milliseconds receive_timeout;
        std::chrono::milliseconds publish_timeout;

    };

    MQTTConnectionOptions::MQTTConnectionOptions() : p(new Priv)
    { }

    MQTTConnectionOptions::~MQTTConnectionOptions() = default;

    MQTTConnectionOptions::MQTTConnectionOptions(const MQTTConnectionOptions & o) : p(new Priv(*o.p))
    { }

    MQTTConnectionOptions & MQTTConnectionOptions::operator = (const MQTTConnectionOptions & o)
    {
        *p = *o.p;
        return *this;
   }

    const std::string & MQTTConnectionOptions::clientId() const
    {
        return p->clientId;
    }

    void MQTTConnectionOptions::setClientId(const std::string & v)
    {
        p->clientId = v;
    }

    const std::string & MQTTConnectionOptions::serverUri() const
    {
        return p->serverUri;
    }

    void MQTTConnectionOptions::setServerUri(const std::string & v)
    {
        p->serverUri = v;
    }

    std::chrono::seconds MQTTConnectionOptions::keepalive() const
    {
        return p->keepalive;
    }

    void MQTTConnectionOptions::setKeepalive(std::chrono::seconds v) const
    {
        p->keepalive = v;
    }

    const std::string & MQTTConnectionOptions::username() const
    {
        return p->username;
    }

    void MQTTConnectionOptions::setUsername(const std::string & v) const
    {
        p->username = v;
    }

    const std::string & MQTTConnectionOptions::password() const
    {
        return p->password;
    }

    void MQTTConnectionOptions::setPassword(const std::string & v) const
    {
        p->password = v;
    }

    bool MQTTConnectionOptions::cleanSession() const
    {
        return p->cleanSession;
    }

    void MQTTConnectionOptions::setCleanSession(bool v) const
    {
        p->cleanSession = v;
    }

    std::chrono::seconds MQTTConnectionOptions::connectTimeout() const
    {
        return p->connectTimeout;
    }

    void MQTTConnectionOptions::setConnectTimeout(std::chrono::seconds v) const
    {
        p->connectTimeout = v;
    }

    std::chrono::seconds MQTTConnectionOptions::retryInterval() const
    {
        return p->retryInterval;
    }

    void MQTTConnectionOptions::setRetryInterval(std::chrono::seconds v) const
    {
        p->retryInterval = v;
    }

    std::chrono::milliseconds MQTTConnectionOptions::receiveTimeout() const
    {
        return p->receive_timeout;
    }

    void MQTTConnectionOptions::setReceiveTimeout(std::chrono::milliseconds v) const
    {
        p->receive_timeout = v;
    }

    std::chrono::milliseconds MQTTConnectionOptions::publishTimeout() const
    {
        return p->publish_timeout;
    }

    void MQTTConnectionOptions::setPublishTimeout(std::chrono::milliseconds v) const
    {
        p->publish_timeout = v;
    }


	bool MQTTConnectionOptions::build(const std::string & config_path, ConfigReader * cr, ErrorCollector & ec)
	{
		SAS_LOG_NDC();

		long long ll_tmp;

        if(!cr->getStringEntry(config_path + "/CLIENT_ID", p->clientId, genRandomString(32), ec))
			return false;

        if(!cr->getStringEntry(config_path + "/SERVER_URI", p->serverUri, "localhost:1883", ec))
			return false;

        if(!cr->getStringEntry(config_path + "/USERNAME", p->username, std::string(), ec))
			return false;

        if(!cr->getStringEntry(config_path + "/PASSWORD", p->password, std::string(), ec))
			return false;

        if(!cr->getBoolEntry(config_path + "/CLEAN_SESSION", p->cleanSession, p->cleanSession, ec))
			return false;

        if(!cr->getNumberEntry(config_path + "/KEEPALIVE", ll_tmp, p->keepalive.count(), ec))
			return false;
        p->keepalive = std::chrono::seconds(ll_tmp);

        if(!cr->getNumberEntry(config_path + "/CONNECT_TIMEOUT", ll_tmp, p->connectTimeout.count(), ec))
			return false;
        p->connectTimeout = std::chrono::seconds(ll_tmp);

        if(!cr->getNumberEntry(config_path + "/RETRY_INTERVAL", ll_tmp, p->retryInterval.count(), ec))
			return false;
        p->retryInterval = std::chrono::seconds(ll_tmp);

		//if(!cr->getNumberEntry(config_path + "/QUS", ll_tmp, 2, ec))
		//	return false;
		//options.qus = (long) ll_tmp;

        if(!cr->getNumberEntry(config_path + "/PUBLISH_TIMEOUT", ll_tmp, p->publish_timeout.count(), ec))
			return false;
        p->publish_timeout = std::chrono::milliseconds(ll_tmp);

        if(!cr->getNumberEntry(config_path + "/RECEIVE_TIMEOUT", ll_tmp, p->receive_timeout.count(), ec))
			return false;
        p->receive_timeout = std::chrono::milliseconds(ll_tmp);

		return true;
	}

}

