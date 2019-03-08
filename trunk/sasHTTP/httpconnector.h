/*
This file is part of sasHTTP.

sasHTTP is free software: you can redistribute it and/or modify
it under the terms of the Lesser GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

sasHTTP is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with sasHTTP.  If not, see <http://www.gnu.org/licenses/>
*/

#ifndef sasHTTP__httpconnector_h
#define sasHTTP__httpconnector_h

#include "config.h"

#include <sasCore/connector.h>

#include <memory>

namespace SAS {

	class Application;

	class HTTPConnector_internal;

	class HTTPConnector : public Connector
	{
		friend class HTTPConnector_internal;
		struct Priv;
		Priv * priv;
	public:
		HTTPConnector(const std::string & name, Application * app);
		virtual ~HTTPConnector();

		virtual std::string name() const final;

		virtual bool init(const std::string & path, ErrorCollector & ec) final;

		virtual bool connect(ErrorCollector & ec) final;

		virtual Connection * createConnection(const std::string & module_name, const std::string & invoker_name, ErrorCollector & ec) final;

		virtual bool getModuleInfo(const std::string & moduleName, std::string & description, std::string & version, ErrorCollector & ec) final;

	};

}

#endif // sasHTTP__httpconnector_h
