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

#ifndef INCLUDE_SASCORE_SESSIONMANAGER_H_
#define INCLUDE_SASCORE_SESSIONMANAGER_H_

#include <chrono>
#include "defines.h"
#include "session.h"

namespace SAS {

	class ErrorCollector;

	struct SessionManager_priv;

	class SAS_CORE__CLASS SessionManager
	{
		SAS_COPY_PROTECTOR(SessionManager)

	public:
		SessionManager();
		virtual ~SessionManager();

		bool init(long default_session_lifetime_secs,  ErrorCollector & ec);
		void deinit();

		Session * getSession(SessionID sid, ErrorCollector & ec);
		void endSession(SessionID sid);

	protected:
		virtual Session * createSession(SessionID id, ErrorCollector & ec) = 0;

	private:
		SessionManager_priv * priv;
	};
}

#endif /* INCLUDE_SASCORE_SESSIONMANAGER_H_ */
