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

#include "corbainterface.h"
#include <sasCore/thread.h>
#include <sasCore/logging.h>
#include <sasCore/errorcodes.h>
#include <omniORB4/CORBA.h>

#include "tools.h"

#include <sasCore/errorcollector.h>
#include <sasCore/application.h>
#include <sasCore/objectregistry.h>
#include <sasCore/module.h>
#include <sasCore/configreader.h>

#include <list>
#include <iostream>
#include <numeric>
#include <fstream>
#include <mutex>

#include "generated/corbasas.hh"

namespace SAS {

class CorbaSASModule_impl : public POA_CorbaSAS::SASModule
{
public:
	CorbaSASModule_impl(Application * app) :
		_app(app), _logger(Logging::getLogger("SAS.CorbaSASModuleImpl"))
	{ }

	Logging::LoggerPtr logger() const
	{
		return _logger;
	}

	struct Err
	{
		long code;
		std::string text;
	};

	static CorbaSAS::ErrorHandling::ErrorSequence toErrorSequence(const std::list<Err> & errs)
	{
		CorbaSAS::ErrorHandling::ErrorSequence_var ret = new CorbaSAS::ErrorHandling::ErrorSequence();
		ret->length(errs.size());
		size_t i(0);
		for(const auto & e : errs)
		{
			CorbaSAS::ErrorHandling::Error err;
			err.error_code = e.code;
			err.error_text = CORBA::string_dup(e.text.c_str());
			ret[i++] = err;
		}
		return ret;
	}

    virtual void invoke(CorbaSAS::SASModule::SessionID& session_id, const char* module_name, const char* invoker,
    		const ::CorbaSAS::SASModule::OctetSequence& in_msg, ::CorbaSAS::SASModule::OctetSequence_out out_msg) final
	{
    	SAS_LOG_NDC();

    	std::list<Err> errs;
    	SimpleErrorCollector ec([&](long errorCode, const std::string & errorText)
    		{ errs.push_back({errorCode, errorText}); });

    	Module * module;
    	if(!(module = _app->objectRegistry()->getObject<Module>(SAS_OBJECT_TYPE__MODULE, module_name, ec)))
    		throw CorbaSAS::ErrorHandling::ErrorException(module_name, invoker, toErrorSequence(errs));

    	SessionID sid = session_id;
    	Session * session;
    	if(!(session = module->getSession(sid, ec)))
    		throw CorbaSAS::ErrorHandling::ErrorException(module_name, invoker, toErrorSequence(errs));
    	session_id = session->id();

    	std::vector<char> out;
    	std::vector<char> tmp = CorbaTools::toByteArray(in_msg);
		switch(session->invoke(invoker, tmp, out, ec))
    	{
    	case Invoker::Status::FatalError:
    		throw CorbaSAS::ErrorHandling::FatalErrorException(module_name, invoker, toErrorSequence(errs));
    	case Invoker::Status::Error:
    		throw CorbaSAS::ErrorHandling::ErrorException(module_name, invoker, toErrorSequence(errs));
    	case Invoker::Status::NotImplemented:
    		throw CorbaSAS::ErrorHandling::NotImplementedException(module_name, invoker, toErrorSequence(errs));
    	case Invoker::Status::OK:
    		break;
    	}
    	CorbaTools::toOctetSequence(out, out_msg);
	}

    virtual void endSession(const char * module_name, ::CorbaSAS::SASModule::SessionID session_id) final
	{
    	SAS_LOG_NDC();
    	std::list<Err> errs;
    	SimpleErrorCollector ec([&](long errorCode, const std::string & errorText)
    		{ errs.push_back({errorCode, errorText}); });

    	Module * module;
    	if(!(module = _app->objectRegistry()->getObject<Module>(SAS_OBJECT_TYPE__MODULE, module_name, ec)))
    		throw CorbaSAS::ErrorHandling::ErrorException(module_name, "", toErrorSequence(errs));

    	module->endSession(session_id);
	}

	virtual void getModuleInfo(const char* module_name, ::CORBA::String_out description, ::CORBA::String_out version) final
	{
    	SAS_LOG_NDC();
    	std::list<Err> errs;
    	SimpleErrorCollector ec([&](long errorCode, const std::string & errorText)
    		{ errs.push_back({errorCode, errorText}); });

    	Module * module;
    	if(!(module = _app->objectRegistry()->getObject<Module>(SAS_OBJECT_TYPE__MODULE, module_name, ec)))
    		throw CorbaSAS::ErrorHandling::ErrorException(module_name, "", toErrorSequence(errs));

    	description = CORBA::string_dup(module->description().c_str());
    	version = CORBA::string_dup(module->version().c_str());
	}

	virtual void getSession(CorbaSAS::SASModule::SessionID& session_id, const char* module_name) final
	{
		std::list<Err> errs;
		SimpleErrorCollector ec([&](long errorCode, const std::string & errorText)
		{ errs.push_back({ errorCode, errorText }); });

		Module * module;
		if (!(module = _app->objectRegistry()->getObject<Module>(SAS_OBJECT_TYPE__MODULE, module_name, ec)))
			throw CorbaSAS::ErrorHandling::ErrorException(module_name, CORBA::string_dup(""), toErrorSequence(errs));

		SessionID sid = session_id;
		Session * session;
		if (!(session = module->getSession(sid, ec)))
			throw CorbaSAS::ErrorHandling::ErrorException(module_name, CORBA::string_dup(""), toErrorSequence(errs));
		session_id = session->id();
	}

private:
   Application * _app;
   Logging::LoggerPtr _logger;
};

class CorbaServer
{
public:
	CorbaServer(CorbaInterface * interface, Application * app) :
		_app(app), _interface(interface), corba_sas_module(new CorbaSASModule_impl(app))
	{ }

	struct ServerConnectionInfo
	{
		ServerConnectionInfo() : use_name_service(false) { }

		bool use_name_service;
		std::string service_name, interface_name;
		std::string ior_file;
	};

	bool bindObjectToName(const std::string & service_name, const std::string & interface_name, CORBA::ORB_ptr orb, CORBA::Object_ptr objref, ErrorCollector & ec)
	{
		SAS_LOG_NDC();

		static std::mutex bind_mut;
		std::unique_lock<std::mutex> __bind_mut_locker(bind_mut);

		CosNaming::NamingContext_var rootContext;

		try
		{
			// Obtain a reference to the root context of the Name service:
			CORBA::Object_var obj;
			obj = orb->resolve_initial_references("NameService");

			// Narrow the reference returned.
			SAS_LOG_TRACE(_interface->logger(), "get root context");
			rootContext = CosNaming::NamingContext::_narrow(obj);
			if( CORBA::is_nil(rootContext) )
			{
				auto err = ec.add(SAS_CORE__ERROR__INTERFACE__CANNOT_REGISTER_SERVER, "Failed to narrow the root naming context.");
				SAS_LOG_ERROR(_interface->logger(), err);
				return false;
			}
		}
		catch(CORBA::ORB::InvalidName & ex)
		{
			// This should not happen!
			auto err = ec.add(SAS_CORE__ERROR__INTERFACE__CANNOT_REGISTER_SERVER, "Service required is invalid [does not exist].");
			SAS_LOG_ERROR(_interface->logger(), err);
			CorbaTools::logException(_interface->logger(), ex);
			return false;
		}

		try
		{
			// Bind a context called "test" to the root context:

			CosNaming::Name contextName;
			contextName.length(1);
			contextName[0].id   = (const char*) service_name.c_str();       // string copied
			contextName[0].kind = (const char*) "sas"; // string copied
			// Note on kind: The kind field is used to indicate the type
			// of the object. This is to avoid conventions such as that used
			// by files (name.type -- e.g. test.ps = postscript etc.)

			CosNaming::NamingContext_var serviceContext;
			try
			{
				// Bind the context to root.
				SAS_LOG_TRACE(_interface->logger(), "bind service");
				serviceContext = rootContext->bind_new_context(contextName);
			}
			catch(CosNaming::NamingContext::AlreadyBound &)
			{
				// If the context already exists, this exception will be raised.
				// In this case, just resolve the name and assign testContext
				// to the object returned:
				SAS_LOG_TRACE(_interface->logger(), "service had been already bound, use the existing context");
				CORBA::Object_var obj;
				obj = rootContext->resolve(contextName);
				serviceContext = CosNaming::NamingContext::_narrow(obj);
				if( CORBA::is_nil(serviceContext) )
				{
					auto err = ec.add(SAS_CORE__ERROR__INTERFACE__CANNOT_REGISTER_SERVER, "Failed to narrow naming context.");
					SAS_LOG_ERROR(_interface->logger(), err);
					return false;
				}
			}

			// Bind objref with name Echo to the testContext:
			CosNaming::Name objectName;
			objectName.length(1);
			objectName[0].id   = (const char*) interface_name.c_str();   // string copied
			objectName[0].kind = (const char*) "interface"; // string copied

			try
			{
				SAS_LOG_TRACE(_interface->logger(), "bind interface");
				serviceContext->bind(objectName, objref);
			}
			catch(CosNaming::NamingContext::AlreadyBound &)
			{
				SAS_LOG_TRACE(_interface->logger(), "interface had been already bound, rebind it");
				serviceContext->rebind(objectName, objref);
			}
			// Note: Using rebind() will overwrite any Object previously bound
			//       to /test/Echo with obj.
			//       Alternatively, bind() can be used, which will raise a
			//       CosNaming::NamingContext::AlreadyBound exception if the name
			//       supplied is already bound to an object.

			// Amendment: When using OrbixNames, it is necessary to first try bind
			// and then rebind, as rebind on it's own will throw a NotFoundexception if
			// the Name has not already been bound. [This is incorrect behaviour -
			// it should just bind].
		}
		catch(CORBA::COMM_FAILURE & ex)
		{
			auto err = ec.add(SAS_CORE__ERROR__INTERFACE__COMMUNICATION_FAILURE, "Caught system exception COMM_FAILURE -- unable to contact the naming service.");
			SAS_LOG_ERROR(_interface->logger(), err);
			CorbaTools::logException(_interface->logger(), ex);
			return false;
		}
		catch(CORBA::SystemException & ex)
		{
			auto err = ec.add(SAS_CORE__ERROR__INTERFACE__UNEXPECTED_ERROR, "Caught a CORBA::SystemException while using the naming service.");
			SAS_LOG_ERROR(_interface->logger(), err);
			CorbaTools::logException(_interface->logger(), ex);
			return false;
		}
		catch (CORBA::Exception & ex)
		{
			auto err = ec.add(SAS_CORE__ERROR__INTERFACE__UNEXPECTED_ERROR, "Caught a CORBA::Exception while using the naming service.");
			SAS_LOG_ERROR(_interface->logger(), err);
			CorbaTools::logException(_interface->logger(), ex);
			return false;
		}
		catch (omniORB::fatalException & ex)
		{
			auto err = ec.add(SAS_CORE__ERROR__INTERFACE__UNEXPECTED_ERROR, "Caught omniORB::fatalException");
			SAS_LOG_FATAL(_interface->logger(), err);
			CorbaTools::logException(_interface->logger(), ex);
			return false;
		}
		catch (...)
		{
			auto err = ec.add(SAS_CORE__ERROR__INTERFACE__UNEXPECTED_ERROR, "Caught an unknown while using the naming service.");
			SAS_LOG_FATAL(_interface->logger(), err);
			return false;
		}

		return true;
	}

	bool init(const CORBA::ORB_var & orb, const ServerConnectionInfo & serverConnectionInfo, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		try
		{
			this->orb = orb;
			obj = orb->resolve_initial_references("RootPOA");
			poa = PortableServer::POA::_narrow(obj);

			PortableServer::ObjectId_var corba_sas_module_id = poa->activate_object(corba_sas_module);

			obj = corba_sas_module->_this();

			_serverConnectionInfo = serverConnectionInfo;
		}
		catch(CORBA::SystemException & ex)
		{
			auto err = ec.add(SAS_CORE__ERROR__INTERFACE__UNEXPECTED_ERROR, "Caught CORBA::SystemException.");
			SAS_LOG_ERROR(_interface->logger(), err);
			CorbaTools::logException(_interface->logger(), ex);
			return false;
		}
		catch(CORBA::Exception & ex)
		{
			auto err = ec.add(SAS_CORE__ERROR__INTERFACE__UNEXPECTED_ERROR, "Caught CORBA::Exception.");
			SAS_LOG_ERROR(_interface->logger(), err);
			CorbaTools::logException(_interface->logger(), ex);
			return false;
		}
		catch(omniORB::fatalException & ex)
		{
			auto err = ec.add(SAS_CORE__ERROR__INTERFACE__UNEXPECTED_ERROR, "Caught omniORB::fatalException");
			SAS_LOG_FATAL(_interface->logger(), err);
			CorbaTools::logException(_interface->logger(), ex);
			return false;
		}
		catch(...)
		{
			auto err = ec.add(SAS_CORE__ERROR__INTERFACE__UNEXPECTED_ERROR, "Caught an unknown exception");
			SAS_LOG_FATAL(_interface->logger(), err);
			return false;
		}

		return true;
	}

	Interface::Status run(ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		try
		{
			if(_serverConnectionInfo.use_name_service)
			{
				SAS_LOG_INFO(_interface->logger(), "bind object to name server: '"+_serverConnectionInfo.service_name+"."+_serverConnectionInfo.interface_name+"'");

				if(!bindObjectToName(_serverConnectionInfo.service_name, _serverConnectionInfo.interface_name, orb, obj, ec))
					return Interface::Status::CannotStart;
			}

			if(_serverConnectionInfo.ior_file.length())
			{
				SAS_LOG_INFO(_interface->logger(), "write ior file: '" + _serverConnectionInfo.ior_file + "'");
				std::ofstream f(_serverConnectionInfo.ior_file);
				CORBA::String_var sior(orb->object_to_string(obj));
				f << sior;
				f.close();
			}

			corba_sas_module->_remove_ref();

			PortableServer::POAManager_var pman = poa->the_POAManager();
			pman->activate();

			SAS_LOG_INFO(_interface->logger(), "run corba server: '" + _serverConnectionInfo.service_name + "." + _serverConnectionInfo.interface_name +"'");
			orb->run();
		}
		catch(CORBA::SystemException & ex)
		{
			auto err = ec.add(SAS_CORE__ERROR__INTERFACE__UNEXPECTED_ERROR, "Caught CORBA::SystemException.");
			SAS_LOG_ERROR(_interface->logger(), err);
			CorbaTools::logException(_interface->logger(), ex);
			return Interface::Status::Crashed;
		}
		catch(CORBA::Exception & ex)
		{
			auto err = ec.add(SAS_CORE__ERROR__INTERFACE__UNEXPECTED_ERROR, "Caught CORBA::Exception.");
			SAS_LOG_ERROR(_interface->logger(), err);
			CorbaTools::logException(_interface->logger(), ex);
			return Interface::Status::Crashed;
		}
		catch(omniORB::fatalException & ex)
		{
			auto err = ec.add(SAS_CORE__ERROR__INTERFACE__UNEXPECTED_ERROR, "Caught omniORB::fatalException");
			SAS_LOG_FATAL(_interface->logger(), err);
			CorbaTools::logException(_interface->logger(), ex);
			return Interface::Status::Crashed;
		}
		catch(...)
		{
			auto err = ec.add(SAS_CORE__ERROR__INTERFACE__UNEXPECTED_ERROR, "Caught an unknown exception");
			SAS_LOG_FATAL(_interface->logger(), err);
			return Interface::Status::Crashed;
		}
		SAS_LOG_INFO(_interface->logger(), "interface is stopped");

		return Interface::Status::Ended;
	}

	Interface::Status shutdown(ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		try
		{
			SAS_LOG_INFO(_interface->logger(), "shut down CORBA interface.");
			orb->shutdown(true);
		}
		catch(CORBA::SystemException & ex)
		{
			auto err = ec.add(SAS_CORE__ERROR__INTERFACE__UNEXPECTED_ERROR, "Caught CORBA::SystemException.");
			SAS_LOG_ERROR(_interface->logger(), err);
			CorbaTools::logException(_interface->logger(), ex);
			return Interface::Status::Crashed;
		}
		catch(CORBA::Exception & ex)
		{
			auto err = ec.add(SAS_CORE__ERROR__INTERFACE__UNEXPECTED_ERROR, "Caught CORBA::Exception.");
			SAS_LOG_ERROR(_interface->logger(), err);
			CorbaTools::logException(_interface->logger(), ex);
			return Interface::Status::Crashed;
		}
		catch(omniORB::fatalException & ex)
		{
			auto err = ec.add(SAS_CORE__ERROR__INTERFACE__UNEXPECTED_ERROR, "Caught omniORB::fatalException");
			SAS_LOG_FATAL(_interface->logger(), err);
			CorbaTools::logException(_interface->logger(), ex);
			return Interface::Status::Crashed;
		}
		catch(...)
		{
			auto err = ec.add(SAS_CORE__ERROR__INTERFACE__UNEXPECTED_ERROR, "Caught an unknown exception");
			SAS_LOG_FATAL(_interface->logger(), err);
			return Interface::Status::Crashed;
		}
		return Interface::Status::Stopped;
	}

private:
	ServerConnectionInfo _serverConnectionInfo;

	Application * _app;
	CorbaInterface * _interface;
	CORBA::ORB_var orb;
	CORBA::Object_var obj;
	PortableServer::POA_var poa;
	PortableServer::Servant_var<CorbaSASModule_impl> corba_sas_module;
};


CorbaInterface::CorbaInterface(const std::string & name, Application * app) : SAS::Interface(),
		_app(app),
		_runner(new CorbaServer(this, app)),
		_logger(Logging::getLogger("SAS.CorbaInterface." + name)),
		_name(name)
{ }

std::string CorbaInterface::name() const
{
	return _name;
}

CorbaInterface::Status CorbaInterface::run(ErrorCollector & ec)
{
	SAS_LOG_NDC();
	return _runner->run(ec);
}

CorbaInterface::Status CorbaInterface::shutdown(ErrorCollector & ec)
{
	SAS_LOG_NDC();
	return _runner->shutdown(ec);
}

bool CorbaInterface::init(const CORBA::ORB_var & orb, const std::string & config_path, ErrorCollector & ec)
{
	SAS_LOG_NDC();

	CorbaServer::ServerConnectionInfo info;

	if(!_app->configReader()->getBoolEntry(config_path + "/USE_NAME_SERVER", info.use_name_service, true, ec))
		return false;

	if(info.use_name_service)
	{
		SAS_LOG_TRACE(_logger, "get name server binding information");

		if (!_app->configReader()->getStringEntry("SAS/CORBA/SERVICE_NAME", info.service_name, "SAS", ec) ||
			!_app->configReader()->getStringEntry(config_path + "/SERVICE_NAME", info.service_name, info.service_name, ec))
			return false;

		if (!info.service_name.length())
		{
			auto err = ec.add(SAS_CORE__ERROR__INTERFACE__CANNOT_REGISTER_SERVER, "service name os not configured");
			SAS_LOG_ERROR(_logger, err);
			return false;
		}

		info.interface_name = _name;
	}

	_app->configReader()->getStringEntry(config_path + "/IOR_FILE", info.ior_file, std::string(), ec);

	return _runner->init(orb, info, ec);
}

Logging::LoggerPtr CorbaInterface::logger() const
{
	return _logger;
}

}
