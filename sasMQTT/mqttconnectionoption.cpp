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

#include "mqttconnectionoptions.h"

#include <sasCore/errorcollector.h>
#include <sasCore/configreader.h>
#include <sasCore/tools.h>
#include <sasCore/logging.h>

namespace SAS {

	bool MQTTConnectionOptions::build(const std::string & config_path, ConfigReader * cr, ErrorCollector & ec)
	{
		SAS_LOG_NDC();

		long long ll_tmp;

		if(!cr->getStringEntry(config_path + "/CLIENT_ID", this->clientId, genRandomString(32), ec))
			return false;

		if(!cr->getStringEntry(config_path + "/SERVER_URI", this->serverUri, "localhost:1883", ec))
			return false;

		if(!cr->getStringEntry(config_path + "/UERNAME", this->username, std::string(), ec))
			return false;

		if(!cr->getStringEntry(config_path + "/PASSWORD", this->password, std::string(), ec))
			return false;

		if(!cr->getBoolEntry(config_path + "/CLEAN_SESSION", this->cleanSession, this->cleanSession, ec))
			return false;

		if(!cr->getNumberEntry(config_path + "/KEEPALIVE", ll_tmp, this->keepalive, ec))
			return false;
		this->keepalive = (long) ll_tmp;

		if(!cr->getNumberEntry(config_path + "/CONNECT_TIMEOUT", ll_tmp, this->connectTimeout, ec))
			return false;
		this->connectTimeout = (long) ll_tmp;

		if(!cr->getNumberEntry(config_path + "/RETRY_INTERVAL", ll_tmp, this->retryInterval, ec))
			return false;
		this->retryInterval = (long) ll_tmp;

		//if(!cr->getNumberEntry(config_path + "/QUS", ll_tmp, 2, ec))
		//	return false;
		//options.qus = (long) ll_tmp;

		if(!cr->getNumberEntry(config_path + "/PUBLISH_TIMEOUT", ll_tmp, this->publish_timeout, ec))
			return false;
		this->publish_timeout = (long) ll_tmp;

		if(!cr->getNumberEntry(config_path + "/RECEIVE_TIMEOUT", ll_tmp, this->receive_timeout, ec))
			return false;
		this->receive_timeout = (long) ll_tmp;

		return true;
	}

}

