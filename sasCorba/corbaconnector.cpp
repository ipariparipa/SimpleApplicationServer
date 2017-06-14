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
#include <sasCore/errorcodes.h>

#include <omniORB4/CORBA.h>

#include <sasCore/errorcollector.h>
#include <sasCore/configreader.h>
#include <sasCore/application.h>
#include <sasCore/thread.h>

#include <numeric>
#include <mutex>
#include "tools.h"
#include "generated/corbasas.hh"

namespace SAS {

	class CorbaConnector_internal
	{
	public:
		inline CorbaConnector_internal(CorbaConnector * obj_) : obj(obj_)
		{ }

		bool reconnect(CorbaSAS::SASModule_ptr & corba_sas_module, ErrorCollector &ec);

	private:
		CorbaConnector * obj;
	};


	class CorbaConnection :public Connection
	{
	public:
		CorbaConnection(CorbaConnector * conn_, long maxReconnectRecursion_, const std::string & module, const std::string & invoker, Application * app, const CorbaSAS::SASModule_ptr & corba_sas_module) :
			conn(conn_), maxReconnectRecursion(maxReconnectRecursion_), _module(module), _invoker(invoker), _sessionId(0),
			_corba_sas_module(corba_sas_module), _app(app), _logger(Logging::getLogger("SAS.CorbaConnection." + module + "." + invoker))
		{ }

		virtual ~CorbaConnection()
		{
			SAS_LOG_NDC();
			try
			{
				_corba_sas_module->endSession(CORBA::string_dup(_module.c_str()), _sessionId);
			}
			catch(...)
			{
				SAS_LOG_WARN(_logger, "Caught an unknown exception.");
			}
		}

		virtual bool getSession(ErrorCollector & ec) final
		{
			SAS_LOG_NDC();
			long recusiv_counter(0);
			return getSession_recursive(recusiv_counter, ec);
		}

		virtual Status invoke(const std::vector<char> & input, std::vector<char> & output, ErrorCollector & ec) final
		{
			SAS_LOG_NDC();
			long recusiv_counter(0);
			return invoke_recursive(recusiv_counter, input, output, ec);
		}

	private:

		bool getSession_recursive(long & recursive_counter, ErrorCollector & ec)
		{
			SAS_LOG_NDC();
			if (recursive_counter >= maxReconnectRecursion)
			{
				SAS_LOG_INFO(_logger, "'recursive_coiunter' reached the maximum number '10'");
				return false;
			}

			try
			{
				_corba_sas_module->getSession(_sessionId, CORBA::string_dup(_module.c_str()));
			}
			catch (CorbaSAS::ErrorHandling::ErrorException & ex)
			{
				auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__SERVER__ERROR, "Caught a CorbaSAS::ErrorHandling::ErrorException");
				SAS_LOG_DEBUG(_logger, err);
				CorbaTools::logException(_logger, ex, ec);
				return false;
			}
			catch (CorbaSAS::ErrorHandling::FatalErrorException & ex)
			{
				auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__SERVER__FATAL_ERROR, "Caught a CorbaSAS::ErrorHandling::FatalErrorException");
				SAS_LOG_DEBUG(_logger, err);
				CorbaTools::logException(_logger, ex, ec);
				return false;
			}
			catch (CorbaSAS::ErrorHandling::NotImplementedException & ex)
			{
				auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__SERVER__NOT_IMPLEMENTED, "Caught a CorbaSAS::ErrorHandling::NotImplementedException");
				SAS_LOG_DEBUG(_logger, err);
				CorbaTools::logException(_logger, ex, ec);
				return false;
			}
			catch (CORBA::COMM_FAILURE & ex)
			{
				auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__COMMUNICATION_FAILURE, "Caught a CORBA::COMM_FAILURE");
				SAS_LOG_DEBUG(_logger, err);
				CorbaTools::logException(_logger, ex);
				return false;
			}
			catch (CORBA::SystemException & ex)
			{
				auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__UNEXPECTED_ERROR, "Caught a CORBA::SystemException.");
				SAS_LOG_ERROR(_logger, err);
				CorbaTools::logException(_logger, ex);

				SAS_LOG_INFO(_logger, "client may lost the connection. try to reconnect");
				if (conn->internal().reconnect(_corba_sas_module, ec))
					return getSession_recursive(++recursive_counter, ec);

				return false;
			}
			catch (CORBA::Exception & ex)
			{
				auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__UNEXPECTED_ERROR, "Caught a CORBA::Exception.");
				SAS_LOG_ERROR(_logger, err);
				CorbaTools::logException(_logger, ex);
				return false;
			}
			catch (omniORB::fatalException & ex)
			{
				auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__UNEXPECTED_ERROR, "Caught omniORB::fatalException");
				SAS_LOG_FATAL(_logger, err);
				CorbaTools::logException(_logger, ex);
				return false;
			}
			catch (...)
			{
				auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__UNEXPECTED_ERROR, "Caught an unknown exception.");
				SAS_LOG_FATAL(_logger, err);
				return false;
			}

			return true;
		}

		Status invoke_recursive(long & recursive_counter, const std::vector<char> & input, std::vector<char> & output, ErrorCollector & ec)
		{
			SAS_LOG_NDC();
			if (recursive_counter >= maxReconnectRecursion)
			{
				SAS_LOG_INFO(_logger, "'recursive_coiunter' reached the maximum number '10'");
				return Status::FatalError;
			}

			try
			{
				CorbaSAS::SASModule::OctetSequence_var out;
				_corba_sas_module->invoke(_sessionId,
					CORBA::string_dup(_module.c_str()),
					CORBA::string_dup(_invoker.c_str()),
					CorbaTools::toOctetSequence_var(input), out);
				output = CorbaTools::toByteArray(out);
			}
			catch (CorbaSAS::ErrorHandling::ErrorException & ex)
			{
				auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__SERVER__ERROR, "Caught a CorbaSAS::ErrorHandling::ErrorException");
				SAS_LOG_DEBUG(_logger, err);
				CorbaTools::logException(_logger, ex, ec);
				return Status::Error;
			}
			catch (CorbaSAS::ErrorHandling::FatalErrorException & ex)
			{
				auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__SERVER__FATAL_ERROR, "Caught a CorbaSAS::ErrorHandling::FatalErrorException");
				SAS_LOG_DEBUG(_logger, err);
				CorbaTools::logException(_logger, ex, ec);
				return Status::FatalError;
			}
			catch (CorbaSAS::ErrorHandling::NotImplementedException & ex)
			{
				auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__SERVER__NOT_IMPLEMENTED, "Caught a CorbaSAS::ErrorHandling::NotImplementedException");
				SAS_LOG_DEBUG(_logger, err);
				CorbaTools::logException(_logger, ex, ec);
				return Status::NotImplemented;
			}
			catch (CORBA::COMM_FAILURE & ex)
			{
				auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__COMMUNICATION_FAILURE, "Caught a CORBA::COMM_FAILURE while using the naming service.");
				SAS_LOG_DEBUG(_logger, err);
				CorbaTools::logException(_logger, ex);
				return Status::Error;
			}
			catch (CORBA::SystemException & ex)
			{
				auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__UNEXPECTED_ERROR, "Caught a CORBA::SystemException.");
				SAS_LOG_ERROR(_logger, err);
				CorbaTools::logException(_logger, ex);

				SAS_LOG_INFO(_logger, "client may lost the connection. try to reconnect");
				if (conn->internal().reconnect(_corba_sas_module, ec))
					return invoke_recursive(++recursive_counter, input, output, ec);

				return Status::FatalError;
			}
			catch (CORBA::Exception & ex)
			{
				auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__UNEXPECTED_ERROR, "Caught a CORBA::Exception.");
				SAS_LOG_ERROR(_logger, err);
				CorbaTools::logException(_logger, ex);
				return Status::FatalError;
			}
			catch (omniORB::fatalException & ex)
			{
				auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__UNEXPECTED_ERROR, "Caught omniORB::fatalException");
				SAS_LOG_FATAL(_logger, err);
				CorbaTools::logException(_logger, ex);
				return Status::FatalError;
			}
			catch (...)
			{
				auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__UNEXPECTED_ERROR, "Caught an unknown exception.");
				SAS_LOG_FATAL(_logger, err);
				return Status::FatalError;
			}

			return Status::OK;
		}


		CorbaConnector * conn;
		long maxReconnectRecursion;
		std::string _module, _invoker;
		CORBA::LongLong _sessionId;
		CorbaSAS::SASModule_ptr _corba_sas_module;
		Application * _app;
		Logging::LoggerPtr _logger;
	};

	struct CorbaConnector_priv
	{
		CorbaConnector_priv(CorbaConnector * obj_, const std::string & name_, Application * app_)
			: app(app_), logger(Logging::getLogger("SAS.CorbaConnector." + name_)), name(name_), connectionActive(false), 
			  reconnectSleep(0), maxReconnectNum(0), maxReconnectRecall(0),
			  internal(obj_)
		{ }

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

		std::mutex mut_reconnect;
		bool connectionActive;

		long reconnectSleep;
		long maxReconnectNum;
		long maxReconnectRecall;

		CorbaConnector_internal internal;
	};

	CorbaConnector::CorbaConnector(const std::string & name, Application * app)
		: Connector(), priv(new CorbaConnector_priv(this, name, app))
	{ }

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
				auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__SERVER_NOT_FOUND, "Failed to narrow the root naming context.");
				SAS_LOG_ERROR(logger, err);
				return CORBA::Object::_nil();
			}
		}
		catch(CORBA::ORB::InvalidName & ex)
		{
			// This should not happen!
			auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__SERVER_NOT_FOUND, "Service required is invalid [does not exist].");
			SAS_LOG_ERROR(logger, err);
			CorbaTools::logException(logger, ex);
			return CORBA::Object::_nil();
		}
		catch (CORBA::SystemException & ex)
		{
			auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__UNEXPECTED_ERROR, "Corba::SystemException");
			SAS_LOG_ERROR(logger, err);
			CorbaTools::logException(logger, ex);
			return CORBA::Object::_nil();
		}
		catch (CORBA::Exception & ex)
		{
			auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__UNEXPECTED_ERROR, "Corba::System");
			SAS_LOG_ERROR(logger, err);
			CorbaTools::logException(logger, ex);
			return CORBA::Object::_nil();
		}
		catch (omniORB::fatalException & ex)
		{
			auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__UNEXPECTED_ERROR, "omniORB::fatalException");
			SAS_LOG_FATAL(logger, err);
			CorbaTools::logException(logger, ex);
			return CORBA::Object::_nil();
		}
		catch (...)
		{
			auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__UNEXPECTED_ERROR, "unknown exception");
			SAS_LOG_FATAL(logger, err);
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
		catch(CosNaming::NamingContext::NotFound & ex)
		{
			// This exception is thrown if any of the components of the
			// path [contexts or the object] aren't found:
			auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__SERVER_NOT_FOUND, "Context not found.");
			SAS_LOG_ERROR(logger, err);
			CorbaTools::logException(logger, ex);
		}
		catch(CORBA::COMM_FAILURE & ex)
		{
			auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__COMMUNICATION_FAILURE, "Caught system exception COMM_FAILURE -- unable to contact the naming service.");
			SAS_LOG_ERROR(logger, err);
			CorbaTools::logException(logger, ex);
		}
		catch(CORBA::SystemException & ex)
		{
			auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__UNEXPECTED_ERROR, "Caught a CORBA::SystemException while using the naming service.");
			SAS_LOG_ERROR(logger, err);
			CorbaTools::logException(logger, ex);
		}
		catch (CORBA::Exception & ex)
		{
			auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__UNEXPECTED_ERROR, "Caught a CORBA::Exception while using the naming service.");
			SAS_LOG_ERROR(logger, err);
			CorbaTools::logException(logger, ex);
		}
		catch (omniORB::fatalException & ex)
		{
			auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__UNEXPECTED_ERROR, "Caught omniORB::fatalException");
			SAS_LOG_FATAL(logger, err);
			CorbaTools::logException(logger, ex);
		}
		catch (...)
		{
			auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__UNEXPECTED_ERROR, "Caught an unknown exception while using the naming service.");
			SAS_LOG_FATAL(logger, err);
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
				auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__MISSING_CONNECTION_DATA, "connector '" + priv->name + "' cannot be initializes because of the insufficient connection data");
				SAS_LOG_ERROR(priv->logger, err);
				return false;
			}
		}

		long long tmp_ll;
		if (!priv->app->configReader()->getNumberEntry(config_path + "/MAX_RECONNECT_NUM", tmp_ll, 10, ec))
			return false;
		if (tmp_ll < 0)
		{
			SAS_LOG_WARN(priv->logger, "negative value is not supported for MAX_RECONNECT_NUM, use default value instead (10)");
			priv->maxReconnectNum = 10;
		}
		else if (tmp_ll == 0)
		{
			SAS_LOG_INFO(priv->logger, "reconnect feature is deactivated (MAX_RECONNECT_NUM = 0)");
			priv->maxReconnectNum = 0;
		}
		else
			priv->maxReconnectNum = (long)tmp_ll;

		if (!priv->app->configReader()->getNumberEntry(config_path + "/MAX_RECONNECT_MAX_RECALL", tmp_ll, priv->maxReconnectNum, ec))
			return false;
		if (tmp_ll < 0)
		{
			SAS_LOG_WARN(priv->logger, "negative value is not supported for MAX_RECONNECT_MAX_RECALL, use default value instead (MAX_RECONNECT_NUM)");
			priv->maxReconnectRecall = priv->maxReconnectNum;
		}
		else if (tmp_ll == 0)
		{
			SAS_LOG_WARN(priv->logger, "no recall is set for reconnect feature (MAX_RECONNECT_MAX_RECALL = 0)");
			priv->reconnectSleep = 0;
		}
		else
			priv->maxReconnectRecall = (long)tmp_ll;

		if (!priv->app->configReader()->getNumberEntry(config_path + "/RECONNECT_DELAY", tmp_ll, 5, ec))
			return false;
		if (tmp_ll < 0)
		{
			SAS_LOG_WARN(priv->logger, "negative value is not supported for RECONNECT_DELAY, use default value instead (5)");
			priv->reconnectSleep = 5000;
		}
		else if (tmp_ll == 0)
		{
			SAS_LOG_INFO(priv->logger, "no delay is set for reconnect feature (RECONNECT_DELAY = 0)");
			priv->reconnectSleep = 0;
		}
		else
			priv->reconnectSleep = (long)tmp_ll * 1000;

		priv->orb = orb;

		return true;
	}

	bool CorbaConnector_internal::reconnect(CorbaSAS::SASModule_ptr & corba_sas_module, ErrorCollector &ec)
	{
		SAS_LOG_NDC();
		std::unique_lock<std::mutex> __locker(obj->priv->mut_reconnect);
		obj->priv->connectionActive = false;

		for (int i(0); i < obj->priv->maxReconnectNum; ++i)
		{
			SAS_LOG_INFO(obj->priv->logger, "attempt #" + std::to_string(i+1));
			if (obj->connect(ec))
			{
				corba_sas_module = obj->priv->corba_sas_module;
				return true;
			}

			SAS_LOG_TRACE(obj->priv->logger, "wait for '" + std::to_string(obj->priv->reconnectSleep) + "' milliseconds");
			Thread::sleep(obj->priv->reconnectSleep);
		}
		SAS_LOG_INFO(obj->priv->logger, "was not possible to connect to the server");
		return false;
	}

	bool CorbaConnector::connect(ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		if (priv->connectionActive)
		{
			SAS_LOG_INFO(priv->logger, "connection has maybe elready created. nothing to do.");
			return true;
		}

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
				auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__INVALID_CONNECTION_DATA, "could not create corba object from IOR");
				SAS_LOG_ERROR(priv->logger, err);
				return false;
			}
		}
		else
		{
			auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__UNEXPECTED_ERROR, "unexpected error: no connection data is found");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}

		try
		{
			if(CORBA::is_nil(priv->corba_sas_module = CorbaSAS::SASModule::_narrow(priv->obj)))
			{
				auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__CANNOT_CONNECT, "could not connect to corba server");
				SAS_LOG_ERROR(priv->logger, err);
				return false;
			}
		}
		catch(CORBA::COMM_FAILURE & ex)
		{
			auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__COMMUNICATION_FAILURE, "unable to connect");
			SAS_LOG_ERROR(priv->logger, err);
			CorbaTools::logException(priv->logger, ex);
			return false;
		}
		catch(CORBA::SystemException & ex)
		{
			auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__UNEXPECTED_ERROR, "Caught CORBA::SystemException.");
			SAS_LOG_ERROR(priv->logger, err);
			CorbaTools::logException(priv->logger, ex);
			return false;
		}
		catch(CORBA::Exception & ex)
		{
			auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__UNEXPECTED_ERROR, "Caught CORBA::Exception.");
			SAS_LOG_ERROR(priv->logger, err);
			CorbaTools::logException(priv->logger, ex);
			return false;
		}
		catch(omniORB::fatalException & ex)
		{
			auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__UNEXPECTED_ERROR, "Caught omniORB::fatalException");
			SAS_LOG_FATAL(priv->logger, err);
			CorbaTools::logException(priv->logger, ex);
			return false;
		}
		catch (...)
		{
			auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__UNEXPECTED_ERROR, "Caught unknown exception");
			SAS_LOG_FATAL(priv->logger, err);
			return false;
		}
		priv->connectionActive = true;
		return true;
	}

	bool CorbaConnector::getModuleInfo(const std::string & moduleName, std::string & description, std::string & version, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		long recusiv_counter(0);
		return getModuleInfo_recursive(recusiv_counter, moduleName, description, version, ec);
	}

	bool CorbaConnector::getModuleInfo_recursive(long & recursive_counter, const std::string & moduleName, std::string & description, std::string & version, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		if (recursive_counter >= priv->maxReconnectRecall)
		{
			SAS_LOG_INFO(priv->logger, "'recursive_counter' reached the maximum number '10'");
			return false;
		}

		try
		{
			CORBA::String_var _description, _version;
			priv->corba_sas_module->getModuleInfo(CORBA::string_dup(moduleName.c_str()), _description, _version);
			description = _description;
			version = _version;
		}
		catch (CORBA::COMM_FAILURE & ex)
		{
			auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__COMMUNICATION_FAILURE, "Caught a CORBA::COMM_FAILURE while using the naming service.");
			SAS_LOG_DEBUG(priv->logger, err);
			CorbaTools::logException(priv->logger, ex);
			return false;
		}
		catch (CorbaSAS::ErrorHandling::ErrorException & ex)
		{
			auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__UNEXPECTED_ERROR, "Caught a CorbaSAS::ErrorHandling::ErrorException while using the naming service.");
			SAS_LOG_DEBUG(priv->logger, err);
			CorbaTools::logException(priv->logger, ex, ec);
			return false;
		}
		catch (CorbaSAS::ErrorHandling::FatalErrorException & ex)
		{
			auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__UNEXPECTED_ERROR, "Caught a CorbaSAS::ErrorHandling::FatalErrorException while using the naming service.");
			SAS_LOG_DEBUG(priv->logger, err);
			CorbaTools::logException(priv->logger, ex, ec);
			return false;
		}
		catch (CorbaSAS::ErrorHandling::NotImplementedException & ex)
		{
			auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__UNEXPECTED_ERROR, "Caught a CorbaSAS::ErrorHandling::NotImplementedException.");
			SAS_LOG_DEBUG(priv->logger, err);
			CorbaTools::logException(priv->logger, ex, ec);
			return false;
		}
		catch (CORBA::SystemException & ex)
		{
			auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__UNEXPECTED_ERROR, "Caught a CORBA::SystemException.");
			SAS_LOG_ERROR(priv->logger, err);
			CorbaTools::logException(priv->logger, ex);

			SAS_LOG_INFO(priv->logger, "client may lost the connection. try to reconnect");
			if (priv->internal.reconnect(priv->corba_sas_module, ec))
				return getModuleInfo_recursive(++recursive_counter, moduleName, description, version, ec);

			return false;
		}
		catch (CORBA::Exception & ex)
		{
			auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__UNEXPECTED_ERROR, "Caught a CORBA::Exception.");
			SAS_LOG_ERROR(priv->logger, err);
			CorbaTools::logException(priv->logger, ex);
			return false;
		}
		catch (omniORB::fatalException & ex)
		{
			auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__UNEXPECTED_ERROR, "Caught omniORB::fatalException");
			SAS_LOG_FATAL(priv->logger, err);
			CorbaTools::logException(priv->logger, ex);
			return false;
		}
		catch (...)
		{
			auto err = ec.add(SAS_CORE__ERROR__CONNECTOR__UNEXPECTED_ERROR, "Caught an unknown exception.");
			SAS_LOG_FATAL(priv->logger, err);
			return false;
		}

		return true;
	}

	Connection * CorbaConnector::createConnection(const std::string & module_name, const std::string & invoker_name, ErrorCollector & ec)
	{
		return new CorbaConnection(this, priv->maxReconnectRecall, module_name, invoker_name, priv->app, priv->corba_sas_module);
	}

	CorbaConnector_internal & CorbaConnector::internal()
	{
		return priv->internal;
	}

}
