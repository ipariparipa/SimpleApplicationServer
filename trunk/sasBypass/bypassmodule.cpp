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

#include "bypassmodule.h"

#include <sasCore/session.h>
#include <sasCore/invoker.h>
#include <sasCore/logging.h>
#include <sasCore/configreader.h>
#include <sasCore/objectregistry.h>
#include <sasCore/application.h>
#include <sasCore/errorcollector.h>
#include <sasCore/connector.h>

#include <map>
#include <mutex>

namespace SAS {

	class BypassInvoker : public Invoker
	{
		SAS_COPY_PROTECTOR(BypassInvoker)
	public:
		inline BypassInvoker(Connection * conn_) : Invoker(), conn(conn_)
		{ }
		
		virtual inline ~BypassInvoker()
		{
			delete conn;
		}

		virtual Status invoke(const std::vector<char> & input, std::vector<char> & output, ErrorCollector & ec) final
		{
			SAS_LOG_NDC();
			return conn->invoke(input, output, ec);
		}

	private:
		Connection * conn;
	};

	class BypassSession : public Session
	{
		SAS_COPY_PROTECTOR(BypassSession)
	public:
		BypassSession(SessionID sid, const std::string & module_name_, Connector * connector_) 
			: Session(sid), module_name(module_name_), connector(connector_), logger(Logging::getLogger("SAS.BypassSession." + module_name_))
		{ }

		virtual ~BypassSession()
		{
			for (auto inv : invokers)
				if (inv.second)
					delete inv.second;
		}

	protected:
		virtual Invoker * getInvoker(const std::string & invoker_name, ErrorCollector & ec) final
		{
			SAS_LOG_NDC();
			std::unique_lock<std::mutex> __lock_invokers;
			auto & inv = invokers[invoker_name];
			if (inv)
				return inv;

			auto conn = connector->createConnection(module_name, invoker_name, ec);
			if (!conn)
			{
				invokers.erase(invoker_name);
				return false;
			}

			return inv = new BypassInvoker(conn);
		}
	private:
		Logging::LoggerPtr logger;
		std::mutex invokers_mut;
		std::map<std::string, Invoker*> invokers;
		std::string module_name;
		Connector * connector;
	};

	struct BypassModule_priv
	{
		BypassModule_priv(const std::string & name_) : name(name_), logger(Logging::getLogger("SAS.BypassModule." + name_))
		{ }

		std::string name;
		std::string version;
		Logging::LoggerPtr logger;

		Connector * connector;
	};

	BypassModule::BypassModule(const std::string & name) : 
		Module(), priv(new BypassModule_priv(name))
	{ }

	BypassModule::~BypassModule()
	{
		delete priv;
	}

	std::string BypassModule::description() const
	{ return std::string(); }

	std::string BypassModule::version() const
	{
		return priv->version;
	}

	std::string BypassModule::name() const
	{
		return priv->name;
	}

	bool BypassModule::init(const std::string & config_path, Application * app, ErrorCollector & ec)
	{
		SAS_LOG_NDC();

		std::string connector_name;
		if (!app->configReader()->getStringEntry(config_path + "/CONNECTOR", connector_name, ec))
		{
			auto err = ec.add(-1, "connector is not specified for bypass module '"+priv->name+"'");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}

		if (!(priv->connector = app->objectRegistry()->getObject<Connector>(SAS_OBJECT_TYPE__CONNECTOR, connector_name, ec)))
			return false;
		
		return priv->connector->connect(ec);
	}

	Session * BypassModule::createSession(SessionID id, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		SAS_LOG_ASSERT(priv->logger, priv->connector, "connector must be initialized");
		return new BypassSession(id, priv->name, priv->connector);
	}

}