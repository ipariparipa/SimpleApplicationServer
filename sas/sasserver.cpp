/*
    This file is part of sas.

    sas is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sas is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sas.  If not, see <http://www.gnu.org/licenses/>
 */

#include "sasserver.h"
#include "version.h"
#include <assert.h>
#include <sasBasics/envconfigreader.h>
#include <sasCore/componentloader.h>
#include <sasCore/component.h>
#include <sasCore/errorcollector.h>
#include <sasCore/logging.h>

#include <mutex>
#include <queue>
#include <memory>

namespace SAS {

struct SASServer_priv
{
	SASServer_priv() : configreader(new EnvConfigReader()), logger(Logging::getLogger("SAS.Server")), wd_logger(Logging::getLogger("SAS.WatchDog"))
	{ }

	std::unique_ptr<ConfigReader> configreader;

	struct Watchdog
	{
		struct IntfMsg
		{
			Interface::Status status;
			Interface * obj;
			std::string text;
		};

		std::mutex intf_messages_mut;
		std::queue<IntfMsg> intf_messages;
		std::mutex suspender_mut;

	} watchdog;

	Logging::LoggerPtr logger, wd_logger;
};

SASServer::SASServer() : Application(), InterfaceManager(), Watchdog(), priv(new SASServer_priv)
{ }

SASServer::~SASServer()
{ delete priv; }

std::string SASServer::version() const
{
	return SAS_SERVER_VERSION;
}

ConfigReader * SASServer::configreader()
{
	return priv->configreader.get();
}

Logging::LoggerPtr SASServer::logger()
{
	return priv->logger;
}

void SASServer::addInterfaceEvent(Interface::Status status, Interface * intf, const std::string & message)
{
	std::unique_lock<std::mutex> __locker(priv->watchdog.intf_messages_mut);
	priv->watchdog.intf_messages.push({status, intf, message});
	priv->watchdog.suspender_mut.unlock();
}

void SASServer::run()
{
	while(true)
	{
		priv->watchdog.suspender_mut.lock();
		priv->watchdog.suspender_mut.unlock();
		std::unique_lock<std::mutex> __locker(priv->watchdog.intf_messages_mut);
		while(priv->watchdog.intf_messages.size())
		{
			auto im = priv->watchdog.intf_messages.front();
			priv->watchdog.intf_messages.pop();
			switch(im.status)
			{
			case Interface::Status::CannotStart:
				SAS_LOG_INFO(priv->wd_logger, std::string("interface '") + im.obj->name() + "' could not be started: '" + im.text + "'");
				break;
			case Interface::Status::Started:
				SAS_LOG_INFO(priv->wd_logger, std::string("interface '") + im.obj->name() + "' is started: '" + im.text + "'");
				break;
			case Interface::Status::Stopped:
				SAS_LOG_WARN(priv->wd_logger, std::string("interface '") + im.obj->name() + "' is stopped: '" + im.text + "'");
				break;
			case Interface::Status::Crashed:
				SAS_LOG_ERROR(priv->wd_logger, std::string("interface '") + im.obj->name() + "' is crashed '" + im.text + "'");
				break;
			}
		}
		priv->watchdog.suspender_mut.lock();
	}
}

}


