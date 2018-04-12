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

#include "include/sasCore/uniqueobjectmanager.h"
#include "include/sasCore/logging.h"
#include "include/sasCore/timerthread.h"

#include <map>
#include <mutex>

namespace SAS {

	struct UniqueObjectManager::Object::Priv
	{
		std::chrono::time_point<std::chrono::high_resolution_clock> lastTached;
		std::chrono::seconds max_idletime;
	};

	UniqueObjectManager::Object::Object() : priv(new Priv)
	{ }

	UniqueObjectManager::Object::~Object()
	{
		delete priv;
	}

	void UniqueObjectManager::Object::setLastTached(const std::chrono::time_point<std::chrono::high_resolution_clock> & v)
	{
		priv->lastTached = v;
	}

	std::chrono::time_point<std::chrono::high_resolution_clock> UniqueObjectManager::Object::lastTached() const
	{
		return priv->lastTached;
	}


	struct UniqueObjectManager::Depot::Priv
	{
		std::map<UniqueId, Object*> data;
		std::mutex mutex;
	};

	UniqueObjectManager::Depot::Depot() : priv(new Priv)
	{ }

	UniqueObjectManager::Depot::~Depot()
	{
		delete priv;
	}

	void UniqueObjectManager::Depot::lock()
	{
		priv->mutex.lock();
	}

	void UniqueObjectManager::Depot::unlock()
	{
		priv->mutex.unlock();
	}

	void UniqueObjectManager::Depot::add(UniqueId id, Object* o)
	{
		priv->data[id] = o;
	}

	const std::map<UniqueId, UniqueObjectManager::Object*> & UniqueObjectManager::Depot::data()
	{
		return priv->data;
	}

	void UniqueObjectManager::Depot::erase(UniqueId id)
	{
		priv->data.erase(id);
	}

	void UniqueObjectManager::Depot::clear()
	{
		priv->data.clear();
	}

	struct UniqueObjectManager::Priv
	{
		Priv(const std::string & name_) :
			logger(Logging::getLogger("SAS.UniqueObjectManager." + name_)),
			name(name_)
		{ }

		Priv() :
			logger(Logging::getLogger("SAS.UniqueObjectManager"))
		{ }

		UniqueObjectManager::Depot depot;

		Logging::LoggerPtr logger;
		std::string name;

		std::chrono::seconds default_max_idletime;
	};

	UniqueObjectManager::UniqueObjectManager(const std::string & name) : priv(new Priv(name))
	{ }

	UniqueObjectManager::UniqueObjectManager() : priv(new Priv)
	{ }

	UniqueObjectManager::~UniqueObjectManager()
	{
		delete priv;
	}

	UniqueObjectManager::Object * UniqueObjectManager::getObject(UniqueId & id /*in-out*/, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		std::unique_lock<Depot> __mutex_locker(priv->depot);
		Object * o = nullptr;
		auto now = std::chrono::system_clock::now();
		if (id)
		{
			SAS_LOG_TRACE(priv->logger, "unique ID is already known");
			SAS_LOG_VAR(priv->logger, id);
			if (priv->depot.data().count(id))
			{
				SAS_LOG_TRACE(priv->logger, "object is found for ID: " + std::to_string(id));
				o = priv->depot.data().at(id);
			}
			else
			{
				SAS_LOG_DEBUG(priv->logger, "unique ID is already not found, create a new object for this ID");
				if (!(o = createObject(id, ec)))
					return nullptr;
				priv->depot.add(id, o);
			}
		}
		else
		{
			SAS_LOG_TRACE(priv->logger, "unique ID is unknown, generate a new one");
			while (priv->depot.data().count(std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count()))
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				now = std::chrono::system_clock::now();
			}
			id = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
			SAS_LOG_VAR(priv->logger, id);
			SAS_LOG_TRACE(priv->logger, "create new item");
			if (!(o = createObject(id, ec)))
			{
				SAS_LOG_TRACE(priv->logger, "could not create new item");
				return nullptr;
			}
			SAS_LOG_TRACE(priv->logger, "new item has been created");
			priv->depot.add(id, o);
		}

		o->setLastTached(now);

		return o;
	}

	UniqueId UniqueObjectManager::getUniqueId(ErrorCollector & ec)
	{
		UniqueId ret = 0;
		if (!getObject(ret, ec))
			return -1;
		return ret;	
	}

	void UniqueObjectManager::unuse(UniqueId id)
	{
		SAS_LOG_NDC();
		std::unique_lock<Depot> __mutex_locker(priv->depot);

		SAS_LOG_VAR(priv->logger, id);
		if (priv->depot.data().count(id))
		{
			SAS_LOG_TRACE(priv->logger, "destroy object");
			destroyObject(priv->depot.data().at(id));
			priv->depot.erase(id);
		}
	}

	void UniqueObjectManager::clear()
	{
		SAS_LOG_NDC();
		for (auto it : priv->depot.data())
		{
			SAS_LOG_TRACE(priv->logger, "destroy object");
			destroyObject(it.second);
		}
		priv->depot.clear();
	}

	//virtual 
	UniqueObjectManager::Object * UniqueObjectManager::createObject(const UniqueId & /*id*/, ErrorCollector & /*ec*/)
	{
		return new Object();
	}

	//virtual 
	void UniqueObjectManager::destroyObject(UniqueObjectManager::Object * it)
	{
		delete it;
	}

	UniqueObjectManager::Depot & UniqueObjectManager::depot()
	{
		return priv->depot;
	}

}
