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

#ifndef INCLUDE_SASCORE_SESSION_H_
#define INCLUDE_SASCORE_SESSION_H_

#include <string>
#include <functional>
#include <chrono>

#include "config.h"
#include "invoker.h"

namespace SAS {

	typedef std::chrono::microseconds::rep SessionID;

	class ErrorCollector;

	struct Session_priv;
	class SAS_CORE__CLASS Session
	{
		SAS_COPY_PROTECTOR(Session)
	public:
		Session(SessionID id);
		virtual ~Session();

		Invoker::Status invoke(const std::string & invoker_name, const std::vector<char> & input, std::vector<char> & output, ErrorCollector & ec);

		bool isActive();

		SessionID id() const;

	protected:
		virtual Invoker * getInvoker(const std::string & name, ErrorCollector & ec) = 0;

	private:
		Session_priv * priv;
	};

}

#endif /* INCLUDE_SASCORE_SESSION_H_ */
