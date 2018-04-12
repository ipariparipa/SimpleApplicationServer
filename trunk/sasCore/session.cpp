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

		std::mutex active_mutex;

		SessionID id;
	};

	Session::Session(SessionID id) : priv(new Session_priv(id))
	{ }

	Session::~Session()
	{
		delete priv;
	}

	Invoker::Status Session::invoke(const std::string & invoker_name, const std::vector<char> & input, std::vector<char> & output, ErrorCollector & ec)
	{
		Invoker * inv;
		if(!(inv = getInvoker(invoker_name, ec)))
			return Invoker::Status::FatalError;
		return inv->invoke(input, output, ec);
	}

	bool Session::isActive()
	{
		if(!priv->active_mutex.try_lock())
			return true;
		priv->active_mutex.unlock();
		return false;
	}

	SessionID Session::id() const
	{
		return priv->id;
	}

	bool Session::try_lock()
	{
		return priv->active_mutex.try_lock();
	}

	void Session::lock()
	{
		priv->active_mutex.lock();
	}

	void Session::unlock()
	{
		priv->active_mutex.unlock();
	}

}
