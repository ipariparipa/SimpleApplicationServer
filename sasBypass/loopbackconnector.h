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

#ifndef LOOPBACKCONNECTOR_H_
#define LOOPBACKCONNECTOR_H_

#include <sasCore/connector.h>

namespace SAS {

	class Application;
	class Module;

	struct LoopbackConnection_priv;
	class LoopbackConnection : public Connection
	{
		SAS_COPY_PROTECTOR(LoopbackConnection)
	public:
		LoopbackConnection(Module * module, const std::string & invoker_name);
		virtual ~LoopbackConnection();

		virtual Status invoke(const std::vector<char> & input, std::vector<char> & output, ErrorCollector & ec) final;

		virtual bool getSession(ErrorCollector & ec) final;

	private:
		LoopbackConnection_priv * priv;
	};


	struct LoopbackConnector_priv;
	class LoopbackConnector : public Connector
	{
		SAS_COPY_PROTECTOR(LoopbackConnector)
	public:
		LoopbackConnector(Application * app, const std::string & name);
		virtual ~LoopbackConnector();

		virtual std::string name() const final;

		virtual bool connect(ErrorCollector & ec) final;

		virtual bool getModuleInfo(const std::string & module_name, std::string & description, std::string & version, ErrorCollector & ec) final;

		virtual Connection * createConnection(const std::string & module_name, const std::string & invoker_name, ErrorCollector & ec) final;

	private:
		LoopbackConnector_priv * priv;
	};

}

#endif /* LOOPBACKCONNECTOR_H_ */
