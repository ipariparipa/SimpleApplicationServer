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

#ifndef sasCore__uniqueobjectmanager_h
#define sasCore__uniqueobjectmanager_h

#include "config.h"
#include "defines.h"

#include <chrono>
#include <string>
#include <map>

namespace SAS {

	class ErrorCollector;

	typedef std::chrono::microseconds::rep UniqueId;

	class SAS_CORE__CLASS UniqueObjectManager
	{
		SAS_COPY_PROTECTOR(UniqueObjectManager)

		struct Priv;
		Priv * priv;

	public:

		class SAS_CORE__CLASS Object
		{
			struct Priv;
			Priv * priv;
		public:
			Object();
			virtual ~Object();
			void setLastTached(const std::chrono::time_point<std::chrono::high_resolution_clock> & v);
			std::chrono::time_point<std::chrono::high_resolution_clock> lastTached() const;
		};

		UniqueObjectManager(const std::string & name);
		UniqueObjectManager();
		virtual ~UniqueObjectManager();

		Object * getObject(UniqueId & id /*in-out*/, ErrorCollector & ec);

		UniqueId getUniqueId(ErrorCollector & ec);

		void unuse(UniqueId id);

		void clear();

		class SAS_CORE__CLASS Depot
		{
//			friend class UniqueObjectManager::Priv;

			struct Priv;
			Priv * priv;
		public:
			Depot();
			~Depot();

			void lock();
			void unlock();

			void add(UniqueId id, Object* o);
			const std::map<UniqueId, Object*> & data();
			void erase(UniqueId id);
			void clear();
		};

		Depot & depot();

	protected:
		virtual Object * createObject(const UniqueId & /*id*/, ErrorCollector & /*ec*/);
		virtual void destroyObject(Object * it);

	};

}

#endif // sasCore__uniqueobjectmanager_h
