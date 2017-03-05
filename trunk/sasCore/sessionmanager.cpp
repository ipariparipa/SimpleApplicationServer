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

namespace SAS {

	struct SessionManager_priv
	{
		SessionManager_priv() : cleaner(this), logger(Logging::getLogger("SAS.SessionManager"))
		{ }

		struct Depot
		{
			Depot() : logger(Logging::getLogger("SAS.SessionManager.Depot"))
			{ }

			struct Item
			{
				Item() : session(nullptr)
				{ }

				Session * session;
				std::chrono::time_point<std::chrono::high_resolution_clock> lastTached;
				std::chrono::seconds max_idletime;
			};

			Logging::LoggerPtr logger;
			std::map<SessionID, Item> data;
			std::mutex mutex;
		} depot;

		std::chrono::seconds default_max_idletime;

		struct Cleaner : public TimerThread
		{
			Cleaner(SessionManager_priv * priv_) : logger(Logging::getLogger("SAS.SessionManager.Cleaner")), priv(priv_)
			{ }

			void shot() override
			{
				SAS_LOG_NDC();
				std::unique_lock<std::mutex> __mutex_locker(priv->depot.mutex);

				std::list<std::pair<SessionID, Session*>> to_be_deleted;

				for(auto & it : priv->depot.data)
				{
					if(!it.second.session->isActive() &&
							(std::chrono::high_resolution_clock::now() - it.second.lastTached) >= it.second.max_idletime)
						to_be_deleted.push_back(std::pair<SessionID, Session*>(it.first, it.second.session));
				}
				for(auto & s : to_be_deleted)
				{
					SAS_LOG_DEBUG(logger, "remove old session: " + std::to_string(s.first));
					priv->depot.data.erase(s.first);
					delete s.second;
				}
			}
			Logging::LoggerPtr logger;

			SessionManager_priv * priv;
		} cleaner;


		Logging::LoggerPtr logger;
	};

	SessionManager::SessionManager() : priv(new SessionManager_priv)
	{ }

	SessionManager::~SessionManager()
	{
		deinit();
		delete priv;
	}

	bool SessionManager::init(long default_max_idletime_secs,  ErrorCollector & ec)
	{
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
		std::unique_lock<std::mutex> __mutex_locker(priv->depot.mutex);
		SAS_LOG_TRACE(priv->logger, "remove all sessions");
		for(auto it : priv->depot.data)
			delete it.second.session;
		priv->depot.data.clear();
	}

	Session * SessionManager::getSession(SessionID sid, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		std::unique_lock<std::mutex> __mutex_locker(priv->depot.mutex);
		SessionManager_priv::Depot::Item * it;
		auto now = std::chrono::system_clock::now();
		if(sid)
		{
			SAS_LOG_TRACE(priv->logger, "session ID is already known");
			SAS_LOG_VAR(priv->logger, sid)
			if(priv->depot.data.count(sid))
			{
				SAS_LOG_TRACE(priv->logger, "session is found for ID: " + std::to_string(sid));
				it =  &priv->depot.data[sid];
			}
			else
			{
				SAS_LOG_DEBUG(priv->logger, "session is already not found, create a new session for this ID");
				it = &priv->depot.data[sid];
				if(!(it->session = createSession(sid, ec)))
				{
					priv->depot.data.erase(std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count());
					return nullptr;
				}
				it->max_idletime = priv->default_max_idletime;
			}
		}
		else
		{
			SAS_LOG_TRACE(priv->logger, "session ID is unknown, generate a new one");
			while(priv->depot.data.count(std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count()))
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				now = std::chrono::system_clock::now();
			}
			sid = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
			SAS_LOG_VAR(priv->logger, sid);
			it = &priv->depot.data[sid];
			if(!(it->session = createSession(sid, ec)))
			{
				SAS_LOG_TRACE(priv->logger, "could not create new session");
				priv->depot.data.erase(sid);
				return nullptr;
			}
			SAS_LOG_TRACE(priv->logger, "new session has been created");
			it->max_idletime = priv->default_max_idletime;
		}
		it->lastTached = now;
		return it->session;
	}

	void SessionManager::endSession(SessionID sid)
	{
		SAS_LOG_NDC();
		std::unique_lock<std::mutex> __mutex_locker(priv->depot.mutex);

		SAS_LOG_VAR(priv->logger, sid);
		if(priv->depot.data.count(sid))
		{
			SAS_LOG_DEBUG(priv->logger, "destroy session");
			delete priv->depot.data[sid].session;
			priv->depot.data.erase(sid);
		}
	}

}
