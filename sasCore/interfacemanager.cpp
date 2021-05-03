/*
    This file is part of sasCore.

    sasCore is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasCore is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasCore.  If not, see <http://www.gnu.org/licenses/>
 */

#include "include/sasCore/interfacemanager.h"
#include "include/sasCore/watchdog.h"
#include "include/sasCore/logging.h"
#include "include/sasCore/thread.h"
#include "include/sasCore/errorcollector.h"
#include "include/sasCore/errorcodes.h"
#include "include/sasCore/tools.h"
#include "include/sasCore/application.h"

#include <mutex>
#include <map>
#include <memory>
#include <list>

namespace SAS {

struct InterfaceManager_priv
{
    InterfaceManager_priv(Application * app) :
        app(app),
        logger(Logging::getLogger("SAS.InterfaceManager"))
	{ }

	class InterfaceThread : public Thread
	{
	public:
        InterfaceThread(ThreadPool * pool, Watchdog * wd, Interface * interface_) : Thread(pool),
            _interface(interface_),
            _wd(wd)
		{ }

		bool stop(ErrorCollector & ec)
		{
			Thread::stop();
			auto st = _interface->shutdown(ec);
			_wd->addInterfaceEvent(st, _interface, st == Interface::Status::Stopped ? "stop interface" : "could not stop interface");
			return st == Interface::Status::Stopped;
		}

	protected:
		void execute() final
		{
			std::list<std::string> errors;
			SimpleErrorCollector ec([&](long errorCode, const std::string & errorText) {
				errors.push_back(ErrorCollector::toString(errorCode, errorText));
			});
			_wd->addInterfaceEvent(Interface::Status::Started, _interface, "start interface");
			auto st = _interface->run(ec);
			_wd->addInterfaceEvent(st, _interface, errors.size() ? str_join(errors, "; ") : "interface has ended");
		}

	private:
		Interface * _interface;
		Watchdog * _wd;
	};

    Application * app;
	std::mutex mut;
	std::map<std::string, std::unique_ptr<InterfaceThread>> threads;

	Logging::LoggerPtr logger;

};

InterfaceManager::InterfaceManager(Application * app) : priv(new InterfaceManager_priv(app))
{ }

InterfaceManager::~InterfaceManager()
{ delete priv; }

bool InterfaceManager::start(ErrorCollector & ec)
{
    (void)ec;
    for(auto & th : priv->threads)
		th.second->start();
	return true;
}

void InterfaceManager::terminate()
{
	SAS_LOG_NDC();
	SAS_LOG_DEBUG(priv->logger, "terminate all interfaces");
	priv->threads.clear();
}

bool InterfaceManager::stop(ErrorCollector & ec)
{
    (void)ec;
    SAS_LOG_NDC();
	SAS_LOG_DEBUG(priv->logger, "send stop signal to interfaces");
	bool has_error(false);
	for(auto & th : priv->threads)
		if(!th.second->stop(ec))
			has_error = true;

	if(has_error)
		SAS_LOG_ERROR(priv->logger, "one or more interface could not be stopped");

	return !has_error;
}

bool InterfaceManager::stopOrTerminate(long timeout, ErrorCollector & ec)
{
    (void)ec;
    SAS_LOG_NDC();
	stop(ec);
	bool ret;
	if(!(ret = wait(timeout, ec)))
	{
		SAS_LOG_DEBUG(priv->logger, "one or more interface could not be stopped properly, going to terminate them now");
	}
	else
	{
		SAS_LOG_DEBUG(priv->logger, "interfaces have been properly ended");
	}
	priv->threads.clear();
	return ret;
}

bool InterfaceManager::wait(long timeout, ErrorCollector & ec)
{
    (void)ec;
    SAS_LOG_NDC();
	bool has_error(false);
	for(auto & th : priv->threads)
	{
		if(!th.second->wait(timeout))
		{
			SAS_LOG_ERROR(priv->logger, "interface '"+th.first+"' could not be ended properly)");
			has_error = true;
		}
	}

	return !has_error;
}

void InterfaceManager::wait()
{
	SAS_LOG_NDC();
	NullEC ec;
	SAS_LOG_SOFT_ASSERT(priv->logger, wait(-1, ec), "infinity wait cannot return with 'false'");
}


bool InterfaceManager::registerInterface(Interface * interface_, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	std::unique_lock<std::mutex> __locker(priv->mut);
	if(priv->threads.count(interface_->name()))
	{
		auto err = ec.add(SAS_CORE__ERROR__INTERFACE_MANAGER__ALREADY_REGISTERED, std::string("interface is already registered: ") + interface_->name());
		SAS_LOG_ERROR(logger(), err);
		return false;
	}
    priv->threads[interface_->name()].reset(new InterfaceManager_priv::InterfaceThread(priv->app->threadPool(), watchdog(), interface_));
	SAS_LOG_DEBUG(logger(), std::string("interface has been registered: ") + interface_->name());
	return true;
}

bool InterfaceManager::registerInterfaces(const std::vector<Interface *> & interfaces, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	std::unique_lock<std::mutex> __locker(priv->mut);
	bool has_error(false);
	for(int i = 0, l = interfaces.size(); i < l; ++i)
	{
		auto intf = interfaces[i];
		if(priv->threads.count(intf->name()))
		{
			auto err = ec.add(SAS_CORE__ERROR__INTERFACE_MANAGER__ALREADY_REGISTERED, std::string("interface is already registered: ") + intf->name());
			SAS_LOG_ERROR(logger(), err);
			has_error = true;
		}
		else
		{
            priv->threads[intf->name()].reset(new InterfaceManager_priv::InterfaceThread(priv->app->threadPool(), watchdog(), intf));
			SAS_LOG_DEBUG(logger(), std::string("interface has been registered: ") + intf->name());
		}
	}
	return !has_error;
}

}
