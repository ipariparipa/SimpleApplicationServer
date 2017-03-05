/*
    This file is part of sasSQLClient.

    sasSQLClient is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasSQLClient is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasSQLClient.  If not, see <http://www.gnu.org/licenses/>
 */

#include "sc_module.h"
#include <sasCore/logging.h>
#include <sasCore/application.h>
#include <sasCore/configreader.h>
#include <sasCore/objectregistry.h>
#include <sasCore/session.h>
#include <sasCore/invoker.h>
#include <sasSQL/sqlconnector.h>
#include <sasSQL/sqlstatement.h>
#include <sasSQL/sqlvariant.h>

#include <memory>
#include <sstream>
#include <string.h>

namespace SAS { namespace SQLClient {

class SC_Session_Invoker : public Session, public Invoker
{
public:
	SC_Session_Invoker(SQLConnector * conn_, const std::string & mod_name_, SessionID id) : Session(id),
		mod_name(mod_name_), conn(conn_),
		logger(Logging::getLogger("SAS.SQLClient.SC_Session_Invoker." + mod_name_))
	{ }

	virtual ~SC_Session_Invoker()
	{ }

	virtual Status invoke(const std::vector<char> & input, std::vector<char> & output, ErrorCollector & ec) final
	{
		std::string in_str;
		in_str.append(input.data(), input.size());

		std::unique_ptr<SAS::SQLStatement> stmt(conn->createStatement());
		if(!stmt->prepare("select * from table_01", ec))
			return Invoker::Status::Error;

		if(!stmt->exec(ec))
			return Invoker::Status::Error;

		std::stringstream ss;
		std::vector<SQLVariant> data;
		while(stmt->fetch(data, ec))
		{
			for(auto & d : data)
				ss << d.toString() << "\t";
			ss << std::endl;
		}
		std::string out_str(ss.str());
		output.resize(out_str.length());
		memcpy(output.data(), out_str.data(), output.size());
		return Invoker::Status::OK;
	}

protected:
	virtual Invoker * getInvoker(const std::string & name, ErrorCollector & ec) final
	{
		SAS_LOG_NDC();
		if(name != "plain_text")
		{
			auto err = ec.add(-1, "unsupported invoker type: '" + name + "'");
			SAS_LOG_ERROR(logger, err);
			return nullptr;
		}
		return this;
	}

private:
	std::string mod_name;
	SQLConnector * conn;
	Logging::LoggerPtr logger;
};

struct SC_Module_priv
{
	SC_Module_priv() : conn(nullptr)
	{ }

	std::string name;
	Logging::LoggerPtr logger;
	SQLConnector * conn;
};

SC_Module::SC_Module(const std::string & name) : Module(), priv(new SC_Module_priv)
{
	priv->logger = Logging::getLogger("SAS.SQLClient.SC_Module." + (priv->name = name));
}

SC_Module::~SC_Module()
{
	delete priv;
}

std::string SC_Module::description() const
{
	return "SQLClient Module";
}

std::string SC_Module::version() const
{
	return "1.0";
}

std::string SC_Module::name() const
{
	return priv->name;
}

bool SC_Module::init(Application * app, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	long long default_session_lifetime;
	std::string prefix = "SAS/SQL_CLIENT/" + priv->name;
	if(!app->configreader()->getNumberEntry(prefix + "/DEFAULT_SESSION_LIFETIME", default_session_lifetime, 10, ec))
		return false;

	if(!SessionManager::init((long)default_session_lifetime, ec))
		return false;

	std::string conn_name;
	if(!app->configreader()->getStringEntry(prefix + "/SQL_CONNECTOR", conn_name, ec))
	{
		auto err = ec.add(-1, "connector name is not configured");
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}

	if(!(priv->conn = app->objectRegistry()->getObject<SQLConnector>(SAS_OBJECT_TYPE_SQL__CONNECTOR, conn_name, ec)))
		return false;

	if(!priv->conn->connect(ec))
		return false;

	return true;
}

Session * SC_Module::createSession(SessionID id, ErrorCollector & ec)
{
	return new SC_Session_Invoker(priv->conn, priv->name, id);
}

}}
