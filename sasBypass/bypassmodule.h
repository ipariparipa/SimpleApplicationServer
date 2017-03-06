/*
	This file is part of sasBypass.

	sasBypass is free software: you can redistribute it and/or modify
	it under the terms of the Lesser GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	sasBypass is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with sasBypass.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef sasBypass__bypassmodule_h
#define sasBypass__bypassmodule_h

#include "config.h"
#include <sasCore/module.h>

namespace SAS {
	class Application;

	struct BypassModule_priv;
	class BypassModule : public Module
	{
		SAS_COPY_PROTECTOR(BypassModule)
	public:
		BypassModule(const std::string & name);
		virtual ~BypassModule();

		virtual std::string description() const final;
		virtual std::string version() const final;
		virtual std::string name() const final;

		bool init(const std::string & config_path, Application * app, ErrorCollector & ec);

	protected:
		virtual Session * createSession(SessionID id, ErrorCollector & ec) final;

	private:
		BypassModule_priv * priv;
	};

}

#endif // sasBypass__bypassmodule_h
