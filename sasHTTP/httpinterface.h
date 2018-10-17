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

#ifndef sasHTTP__mqttinterface_h
#define sasHTTP__mqttinterface_h

#include "config.h"

#ifdef SAS_HTTP__HAVE_MICROHTTPD

#include <sasCore/interface.h>
#include <sasCore/logging.h>

#include <string>
#include <memory>

namespace SAS {

	class Application;
	class ErrorCollector;
	
	class HTTPInterface : public Interface
	{
		struct Priv;
		Priv * priv;
	public:
		HTTPInterface(const std::string & name, Application * app);
		virtual ~HTTPInterface();

		virtual std::string name() const final;
		virtual Status run(ErrorCollector & ec) final;
		virtual Status shutdown(ErrorCollector & ec) final;

		bool init(const std::string & config_path, ErrorCollector & ec);

		Logging::LoggerPtr logger() const;
	};

}

#endif // SAS_HTTP__HAVE_MICROHTTPD

#endif // sasHTTP__mqttinterface_h
