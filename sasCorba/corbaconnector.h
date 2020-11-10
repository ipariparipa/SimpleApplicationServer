/*
    This file is part of sasCorba.

    sasCorba is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasCorba is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasCorba.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef CORBACONNECTOR_H_
#define CORBACONNECTOR_H_

#include "config.h"

#include <sasCore/connector.h>
#include <omniORB4/CORBA.h>

namespace SAS {

class Application;

class CorbaConnector_internal;

struct CorbaConnector_priv;

class CorbaConnector : public Connector
{
	friend class CorbaConnector_internal;
public:
	CorbaConnector(const std::string & name, Application * app);
	virtual ~CorbaConnector();

	virtual std::string name() const final;

    virtual bool init(const CORBA::ORB_var & orb, const std::string & configPath, ErrorCollector & ec) final;
    virtual bool init(const CORBA::ORB_var & orb, const std::string & connectionString, const std::string & configPath, ErrorCollector & ec) final;

	virtual bool connect(ErrorCollector & ec) final;

	virtual Connection * createConnection(const std::string & module_name, const std::string & invoker_name, ErrorCollector & ec) final;

	virtual bool getModuleInfo(const std::string & moduleName, std::string & description, std::string & version, ErrorCollector & ec) final;

	CorbaConnector_internal & internal();
private:
	bool getModuleInfo_recursive(long & recursive_counter, const std::string & moduleName, std::string & description, std::string & version, ErrorCollector & ec);

	CorbaConnector_priv * priv;
};

}

#endif /* CORBACONNECTOR_H_ */
