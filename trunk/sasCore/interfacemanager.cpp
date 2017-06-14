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

#include <mutex>
#include <map>
#include <memory>

namespace SAS {

struct InterfaceManager_priv
{
	class InterfaceThread : public Thread
	{
	public:
		InterfaceThread(Watchdog * wd, Interface * interface) : _interface(interface), _wd(wd)
		{ }

	protected:
		void execute() final
		{
			struct StrErrorCollector : public ErrorCollector
			{
				virtual void append(long errorCode, const std::string & errorText)
				{
					str = toString(errorCode, errorText);
				}

				std::string str;
			};

			_wd->addInterfaceEvent(Interface::Status::Started, _interface, std::string());
			std::string err;
			StrErrorCollector ec;
			auto st = _interface->run(ec);
			_wd->addInterfaceEvent(st, _interface, ec.str);
		}

	private:
		Interface * _interface;
		Watchdog * _wd;
	};

	std::mutex mut;
	std::map<std::string, std::unique_ptr<InterfaceThread>> threads;

};

InterfaceManager::InterfaceManager() : priv(new InterfaceManager_priv)
{ }

InterfaceManager::~InterfaceManager()
{ delete priv; }

bool InterfaceManager::start(ErrorCollector & ec)
{
	for(auto & th : priv->threads)
		th.second->start();
	return true;
}

void InterfaceManager::terminate()
{
	priv->threads.clear();
}

bool InterfaceManager::registerInterface(Interface * interface, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	std::unique_lock<std::mutex> __locker(priv->mut);
	if(priv->threads.count(interface->name()))
	{
		auto err = ec.add(SAS_CORE__ERROR__INTERFACE_MANAGER__ALREADY_REGISTERED, std::string("interface is already registered: ") + interface->name());
		SAS_LOG_ERROR(logger(), err);
		return false;
	}
	priv->threads[interface->name()].reset(new InterfaceManager_priv::InterfaceThread(watchdog(), interface));
	SAS_LOG_DEBUG(logger(), std::string("interface has been registered: ") + interface->name());
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
			priv->threads[intf->name()].reset(new InterfaceManager_priv::InterfaceThread(watchdog(), intf));
			SAS_LOG_DEBUG(logger(), std::string("interface has been registered: ") + intf->name());
		}
	}
	return !has_error;
}

}
