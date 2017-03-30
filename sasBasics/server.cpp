/*
    This file is part of sasBasics.

    sasBasics is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasBasics is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasBasics.  If not, see <http://www.gnu.org/licenses/>
 */

#include "include/sasBasics/server.h"
#include <sasCore/componentloader.h>
#include <sasCore/component.h>
#include <sasCore/errorcollector.h>
#include <sasCore/logging.h>

#include <assert.h>

#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>

namespace SAS {

	struct Server_priv
{
	Server_priv() : logger(Logging::getLogger("SAS.Server")), wd_logger(Logging::getLogger("SAS.WatchDog"))
	{ }

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
		std::condition_variable suspender_cv;
		std::mutex suspender_cv_mut;

	} watchdog;

	Logging::LoggerPtr logger, wd_logger;
};

Server::Server(int argc, char ** argv) : 
	Application(argc, argv), InterfaceManager(), Watchdog(), priv(new Server_priv)
{ }

Server::~Server()
{ delete priv; }

Logging::LoggerPtr Server::logger()
{
	return priv->logger;
}

void Server::addInterfaceEvent(Interface::Status status, Interface * intf, const std::string & message)
{
	std::lock_guard<std::mutex> __susp_locker(priv->watchdog.suspender_cv_mut);
	priv->watchdog.intf_messages.push({status, intf, message});
	priv->watchdog.suspender_cv.notify_one();
}

void Server::run()
{
	while(true)
	{
		std::unique_lock<std::mutex> __susp_locker(priv->watchdog.suspender_cv_mut);
		priv->watchdog.suspender_cv.wait(__susp_locker);
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
		priv->watchdog.suspender_cv.notify_one();
	}
}

}
