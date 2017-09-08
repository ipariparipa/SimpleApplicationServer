/*
This file is part of sasMQTT.

sasMQTT is free software: you can redistribute it and/or modify
it under the terms of the Lesser GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

sasMQTT is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with sasMQTT.  If not, see <http://www.gnu.org/licenses/>
*/

#ifndef sasMQTT__mqttinterface_h
#define sasMQTT__mqttinterface_h

#include "config.h"

#include <sasCore/interface.h>
#include <sasCore/logging.h>

#include <string>
#include <memory>

namespace SAS {

	class Application;
	class ErrorCollector;
	class MQTTRunner;
	
	class MQTTInterface : public Interface
	{
	public:
		MQTTInterface(const std::string & name, Application * app);

		virtual std::string name() const final;
		virtual Status run(ErrorCollector & ec) final;

		bool init(const std::string & config_path, ErrorCollector & ec);

		Logging::LoggerPtr logger() const;

	private:
		Application * _app;
		std::unique_ptr<MQTTRunner> _runner;
		Logging::LoggerPtr _logger;
		std::string _name;
	};

}

#endif // sasMQTT__mqttinterface_h
