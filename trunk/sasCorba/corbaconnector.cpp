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

#include "corbaconnector.h"
#include <sasCore/logging.h>

#include <omniORB4/CORBA.h>

#include <sasCore/errorcollector.h>
#include <sasCore/configreader.h>
#include <sasCore/application.h>

#include <numeric>
#include "tools.h"
#include "generated/corbasas.hh"

namespace SAS {

class CorbaConnection :public Connection
{
public:
	CorbaConnection(const std::string & module, const std::string & invoker, Application * app, const CorbaSAS::SASModule_ptr & corba_sas_module) :
		_module(module), _invoker(invoker), _sessionId(0),
		_corba_sas_module(corba_sas_module), _app(app), _logger(Logging::getLogger("SAS.CorbaConnection." + module + "." + invoker))
	{ }

	virtual ~CorbaConnection()
	{
		try
		{
			_corba_sas_module->endSession(CORBA::string_dup(_module.c_str()), _sessionId);
		}
		catch(...)
		{ }
	}

	virtual Status invoke(const std::vector<char> & input, std::vector<char> & output, ErrorCollector & ec) final
	{
		SAS_LOG_NDC();
		try
		{
			CorbaSAS::SASModule::OctetSequence_var out;
			_corba_sas_module->invoke(_sessionId,
					CORBA::string_dup(_module.c_str()),
					CORBA::string_dup(_invoker.c_str()),
					CorbaTools::toOctetSequence_var(input), out);
			output = CorbaTools::toByteArray(out);
		}
		catch(CorbaSAS::ErrorHandling::ErrorException & ex)
		{
			auto err= ec.add(-1, "Caught a CorbaSAS::ErrorHandling::ErrorException while using the naming service.");
			SAS_LOG_DEBUG(_logger, err );
			SAS_LOG_VAR(_logger, ex.invoker);
			SAS_LOG_VAR(_logger, ex.sas_module);
			for(size_t i(0), l(ex.err.length()); i < l; ++i)
			{
				auto err = ec.add(ex.err[i].error_code, ex.err[i].error_text._ptr);
				SAS_LOG_ERROR(_logger, err);
			}
			return Status::Error;
		}
		catch(CorbaSAS::ErrorHandling::FatalErrorException & ex)
		{
			auto err = ec.add(-1, "Caught a CorbaSAS::ErrorHandling::FatalErrorException while using the naming service.");
			SAS_LOG_DEBUG(_logger, err);
			SAS_LOG_VAR(_logger, ex.invoker);
			SAS_LOG_VAR(_logger, ex.sas_module);
			for(size_t i(0), l(ex.err.length()); i < l; ++i)
			{
				auto err = ec.add(ex.err[i].error_code, ex.err[i].error_text._ptr);
				SAS_LOG_ERROR(_logger, err);
			}
			return Status::FatalError;
		}
		catch(CorbaSAS::ErrorHandling::NotImplementedException & ex)
		{
			auto err = ec.add(-1, "Caught a CorbaSAS::ErrorHandling::NotImplementedException.");
			SAS_LOG_DEBUG(_logger, err);
			SAS_LOG_VAR(_logger, ex.invoker);
			SAS_LOG_VAR(_logger, ex.sas_module);
			for(size_t i(0), l(ex.err.length()); i < l; ++i)
			{
				auto err = ec.add(ex.err[i].error_code, ex.err[i].error_text._ptr);
				SAS_LOG_ERROR(_logger, err);
			}
			return Status::NotImplemented;
		}
		catch(CORBA::COMM_FAILURE &)
		{
			auto err = ec.add(-1, "Caught system exception COMM_FAILURE.");
			SAS_LOG_ERROR(_logger, err);
			return Status::FatalError;
		}
		catch(CORBA::SystemException &)
		{
			auto err = ec.add(-1, "Caught a CORBA::SystemException.");
			SAS_LOG_ERROR(_logger, err);
			return Status::FatalError;
		}
		catch (CORBA::Exception &)
		{
			auto err = ec.add(-1, "Caught a CORBA::Exception.");
			SAS_LOG_ERROR(_logger, err);
			return Status::FatalError;
		}
		catch (...)
		{
			auto err = ec.add(-1, "Caught an unknown exception.");
			SAS_LOG_ERROR(_logger, err);
			return Status::FatalError;
		}

		return Status::OK;
	}

private:
	std::string _module, _invoker;
	CORBA::LongLong _sessionId;
	CorbaSAS::SASModule_ptr _corba_sas_module;
	Application * _app;
	Logging::LoggerPtr _logger;
};

struct CorbaConnector_priv
{
	Application * app;
	Logging::LoggerPtr logger;
	std::string name;

	struct ConnectionData
	{
		ConnectionData() : use_name_server(false) { }

		std::string service_name, interface_mame;
		std::string ior;
		bool use_name_server;
	} connectionData;

	CORBA::ORB_var orb;
	CORBA::Object_var obj;
	CorbaSAS::SASModule_ptr corba_sas_module;
};

CorbaConnector::CorbaConnector(const std::string & name, Application * app)
	: Connector(), priv(new CorbaConnector_priv)
{
	priv->name = name;
	priv->app = app;
	priv->logger = Logging::getLogger("SAS.CorbaConnector." + name);
}

CorbaConnector::~CorbaConnector()
{ delete priv; }

std::string CorbaConnector::name() const
{
	return priv->name;
}

CORBA::Object_ptr getObjectReference(const std::string & service_name, const std::string & interface_name, CORBA::ORB_ptr orb, Logging::LoggerPtr logger, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	CosNaming::NamingContext_var rootContext;

	try
	{
		// Obtain a reference to the root context of the Name service:
		CORBA::Object_var obj;
		obj = orb->resolve_initial_references("NameService");

		// Narrow the reference returned.
		rootContext = CosNaming::NamingContext::_narrow(obj);
		if( CORBA::is_nil(rootContext) )
		{
			auto err = ec.add(-1, "Failed to narrow the root naming context.");
			SAS_LOG_ERROR(logger, err);
			return CORBA::Object::_nil();
		}
	}
	catch(CORBA::ORB::InvalidName &)
	{
		// This should not happen!
		auto err = ec.add(-1, "Service required is invalid [does not exist].");
		SAS_LOG_ERROR(logger, err);
		return CORBA::Object::_nil();
	}
	catch (CORBA::SystemException &)
	{
		auto err = ec.add(-1, "Corba::SystemException");
		SAS_LOG_ERROR(logger, err);
		return CORBA::Object::_nil();
	}

	// Create a name object, containing the name test/context:
	CosNaming::Name name;
	name.length(2);

	name[0].id   = (const char*) service_name.c_str();       // string copied
	name[0].kind = (const char*) "sas"; // string copied
	name[1].id   = (const char*) interface_name.c_str();
	name[1].kind = (const char*) "interface";
	// Note on kind: The kind field is used to indicate the type
	// of the object. This is to avoid conventions such as that used
	// by files (name.type -- e.g. test.ps = postscript etc.)

	try
	{
		// Resolve the name to an object reference.
		return rootContext->resolve(name);
	}
	catch(CosNaming::NamingContext::NotFound &)
	{
		// This exception is thrown if any of the components of the
		// path [contexts or the object] aren't found:
		auto err = ec.add(-1, "Context not found.");
		SAS_LOG_ERROR(logger, err);
	}
	catch(CORBA::COMM_FAILURE &)
	{
		auto err = ec.add(-1, "Caught system exception COMM_FAILURE -- unable to contact the naming service.");
		SAS_LOG_ERROR(logger, err);
	}
	catch(CORBA::SystemException &)
	{
		auto err = ec.add(-1, "Caught a CORBA::SystemException while using the naming service.");
		SAS_LOG_ERROR(logger, err);
	}

	return CORBA::Object::_nil();
}

bool CorbaConnector::init(const CORBA::ORB_var & orb, const std::string & config_path, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	std::vector<std::string> ops;

	SAS_LOG_VAR_NAME(priv->logger, "connector_name", priv->name);

	if(!priv->app->configReader()->getBoolEntry(config_path + "/USE_NAME_SERVER", priv->connectionData.use_name_server, true, ec))
		return false;

	if(priv->connectionData.use_name_server)
	{
		SAS_LOG_TRACE(priv->logger, "get name server connection info");
		if(!priv->app->configReader()->getStringEntry(config_path + "/SERVICE_NAME", priv->connectionData.service_name, ec) || !priv->connectionData.service_name.length())
		{
			SAS_LOG_TRACE(priv->logger, "service name is not defined for, get default value");
			if(!priv->app->configReader()->getStringEntry("SAS/CORBA/SERVICE_NAME", priv->connectionData.service_name, ec) || !priv->connectionData.service_name.length())
			{
				SAS_LOG_DEBUG(priv->logger, "default service name is not set");
				priv->connectionData.service_name = "SAS";
			}
		}
		if(!priv->app->configReader()->getStringEntry(config_path + "/INTERFACE_NAME", priv->connectionData.interface_mame, ec) || !priv->connectionData.interface_mame.length())
		{
			SAS_LOG_WARN(priv->logger, "interface name is not defined, use connector name instead");
			priv->connectionData.interface_mame = priv->name;
		}
	}
	else
	{
		SAS_LOG_TRACE(priv->logger, "name server is not used");
		if(!priv->app->configReader()->getStringEntry(config_path + "/IOR", priv->connectionData.ior, ec) || !priv->connectionData.ior.length())
		{
			SAS_LOG_DEBUG(priv->logger, "IOR data is not defined");
			auto err = ec.add(-1, "connector '"+priv->name+"' cannot be initializes because of the insufficient connection data");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}
	}

	priv->orb = orb;

	return true;
}

bool CorbaConnector::connect(ErrorCollector & ec)
{
	SAS_LOG_NDC();
	if(priv->connectionData.use_name_server)
	{
		SAS_LOG_INFO(priv->logger, "connect to corba interface: '"+priv->connectionData.service_name+"."+priv->name+"'");
		if(CORBA::is_nil(priv->obj = getObjectReference(priv->connectionData.service_name, priv->connectionData.interface_mame, priv->orb, priv->logger, ec)))
			return false;
	}
	else if(priv->connectionData.ior.length())
	{
		SAS_LOG_INFO(priv->logger, "use IOR data to get server object");
		if(CORBA::is_nil(priv->obj = priv->orb->string_to_object(priv->connectionData.ior.c_str())))
		{
			auto err = ec.add(-1, "could not create corba object from IOR");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}
	}
	else
	{
		auto err = ec.add(-1, "unexpected error: no connection data is found");
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}

	try
	{
		if(CORBA::is_nil(priv->corba_sas_module = CorbaSAS::SASModule::_narrow(priv->obj)))
		{
			auto err = ec.add(-1, "could not connect to corba server");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}
	}
	catch(CORBA::COMM_FAILURE &)
	{
		auto err = ec.add(-1, "unable to connect");
		SAS_LOG_ERROR(priv->logger, err);
	}
	catch(CORBA::SystemException & ex)
	{
		auto err = ec.add(-1, "Caught CORBA::SystemException.");
		SAS_LOG_ERROR(priv->logger, err);
		SAS_LOG_VAR(priv->logger, ex._rep_id());
		SAS_LOG_VAR(priv->logger, ex.minor());
		SAS_LOG_VAR(priv->logger, ex.completed());
		return false;
	}
	catch(CORBA::Exception &)
	{
		auto err = ec.add(-1, "Caught CORBA::Exception.");
		SAS_LOG_ERROR(priv->logger, err);
	}
	catch(omniORB::fatalException & ex)
	{
		auto err = ec.add(-1, "Caught omniORB::fatalException");
		SAS_LOG_ERROR(priv->logger, err);
		SAS_LOG_VAR(priv->logger, ex.file());
		SAS_LOG_VAR(priv->logger, ex.line());
		SAS_LOG_VAR(priv->logger, ex.errmsg());
	}
	catch(...)
	{
		auto err = ec.add(-1, "Caught unknown exception");
		SAS_LOG_ERROR(priv->logger, err);
	}
	return true;
}

Connection * CorbaConnector::createConnection(const std::string & module_name, const std::string & invoker_name, ErrorCollector & ec)
{
	return new CorbaConnection(module_name, invoker_name, priv->app, priv->corba_sas_module);
}

}
