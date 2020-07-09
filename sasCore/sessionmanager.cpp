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

#include "include/sasCore/sessionmanager.h"

#include <assert.h>
#include <map>
#include <chrono>
#include <mutex>
#include <list>
#include "include/sasCore/errorcollector.h"
#include "include/sasCore/session.h"
#include "include/sasCore/timerthread.h"
#include "include/sasCore/logging.h"
#include "include/sasCore/uniqueobjectmanager.h"

#include <iostream>
#include <sstream>

namespace SAS {

	struct SessionManager::Priv
	{
		Priv(UniqueObjectManager * that_) : that(that_), cleaner(this), logger(Logging::getLogger("SAS.SessionManager"))
		{ }

		struct SessionObject : public UniqueObjectManager::Object
		{
			SessionObject(Session * session_, std::chrono::seconds max_idletime_) : session(session_), max_idletime(max_idletime_)
			{ }

			virtual ~SessionObject()
			{
				assert(session);	
				delete session;
			}

			Session * session;

			std::chrono::seconds max_idletime;
		};

		UniqueObjectManager * that;
		std::chrono::seconds default_max_idletime;

		struct Cleaner : public TimerThread
		{
			Cleaner(Priv * priv_) : logger(Logging::getLogger("SAS.SessionManager.Cleaner")), priv(priv_)
			{ }

			void shot() override
			{
				SAS_LOG_NDC();

				std::list<std::pair<SessionID, SessionObject*>> to_be_deleted;

				{
					auto & depot = priv->that->depot();
					std::unique_lock<UniqueObjectManager::Depot> __locker(depot);
					for (const auto & it : depot.data())
					{
						auto so = dynamic_cast<SessionObject*>(it.second);
						assert(so);
						if (so->session->try_lock())
						{
							if (std::chrono::high_resolution_clock::now() - so->lastTouched() >= so->max_idletime)
								to_be_deleted.push_back(std::pair<SessionID, SessionObject*>(it.first, so));
							so->session->unlock();
						}
					}
					for (auto & s : to_be_deleted)
						depot.erase(s.first);
				}


				for(auto & s : to_be_deleted)
				{
					SAS_LOG_DEBUG(logger, "delete old session: " + std::to_string(s.first));
					delete s.second;
				}
			}
			Logging::LoggerPtr logger;

			Priv * priv;
		} cleaner;


		Logging::LoggerPtr logger;
	};

	SessionManager::SessionManager() : UniqueObjectManager(), priv(new Priv(this))
	{ }

	SessionManager::~SessionManager()
	{
		deinit();
		delete priv;
	}

	bool SessionManager::init(long default_max_idletime_secs,  ErrorCollector & ec)
	{
        (void)ec;
        SAS_LOG_NDC();
		SAS_LOG_VAR(priv->logger, default_max_idletime_secs);
		priv->default_max_idletime = std::chrono::seconds(default_max_idletime_secs);
		SAS_LOG_INFO(priv->logger, "start session cleaner thread");
		priv->cleaner.start(SAS_SESSION_CLEANER_INTERVAL);
		return true;
	}

	void SessionManager::deinit()
	{
		SAS_LOG_NDC();
		SAS_LOG_INFO(priv->logger, "stop session cleaner thread");
		priv->cleaner.stop();
		priv->cleaner.wait();
		SAS_LOG_TRACE(priv->logger, "session cleaner thread has been ended");
		SAS_LOG_TRACE(priv->logger, "remove all sessions");
		clear();
	}

	Session * SessionManager::getSession(SessionID sid, ErrorCollector & ec)
	{
		SAS_LOG_NDC();

		Object * o;
		if (!(o = getObject(sid, ec)))
			return nullptr;

		auto so = dynamic_cast<Priv::SessionObject*>(o);
		assert(so);

		SAS_LOG_TRACE(priv->logger, "lock session");
		so->session->lock();
		return so->session;
	}

	void SessionManager::endSession(SessionID sid)
	{
		SAS_LOG_NDC();
		unuse(sid);
	}

	SessionManager::Object * SessionManager::createObject(const UniqueId & id, ErrorCollector & ec)
	{
		Session * s;
		if (!(s = createSession(id, ec)))
			return nullptr;

		return new Priv::SessionObject(s, priv->default_max_idletime);
	}

	void SessionManager::destroyObject(Object * o)
	{
		auto so = dynamic_cast<Priv::SessionObject*>(o);
		assert(so);
		so->session->lock();
		so->session->unlock();
		delete so;
	}

}
