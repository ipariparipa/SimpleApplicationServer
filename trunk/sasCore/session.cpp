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

#include "include/sasCore/session.h"

#include <map>
#include <chrono>
#include <mutex>
#include <sstream>

namespace SAS {

	struct Session_priv
	{
		Session_priv(SessionID id_) : id(id_)
		{ }

//		std::mutex contexts_mutex;
//		std::map<std::string /*name*/, Session::Context*> contexts;

		std::mutex active_mutex;

		SessionID id;
	};

	Session::Session(SessionID id) : priv(new Session_priv(id))
	{ }

	Session::~Session()
	{
		delete priv;
	}

//	Session::Context::~Context()
//	{ }

	Invoker::Status Session::invoke(const std::string & invoker_name, const std::vector<char> & input, std::vector<char> & output, ErrorCollector & ec)
	{
		std::unique_lock<std::mutex> __locker_(priv->active_mutex);

		Invoker * inv;
		if(!(inv = getInvoker(invoker_name, ec)))
			return Invoker::Status::FatalError;
		return inv->invoke(input, output, ec);
	}

//	Session::Context * Session::beginContext(const std::string & name, std::function<Context*()> alloc)
//	{
//		std::unique_lock<std::mutex> __locker_(priv->contexts_mutex);
//		return priv->contexts.count(name) ? priv->contexts[name] : priv->contexts[name] = alloc();
//	}

//	void Session::endContext(const std::string & name)
//	{
//		std::unique_lock<std::mutex> __locker_(priv->contexts_mutex);
//		if(priv->contexts.count(name))
//		{
//			delete priv->contexts[name];
//			priv->contexts.erase(name);
//		}
//	}

	bool Session::isActive()
	{
//		std::unique_lock<std::mutex> __locker_(priv->contexts_mutex);
//		return priv->contexts.size();
		if(!priv->active_mutex.try_lock())
			return true;
		priv->active_mutex.unlock();
		return false;
	}

	SessionID Session::id() const
	{
		return priv->id;
	}


	namespace Logging {

		std::string toString(SessionID v)
		{
			std::stringstream ss;
			ss << v;
			return ss.str();
		}

	}

}
