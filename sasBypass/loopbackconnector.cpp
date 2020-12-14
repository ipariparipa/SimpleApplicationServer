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

#include "loopbackconnector.h"

#include <sasCore/application.h>
#include <sasCore/objectregistry.h>
#include <sasCore/module.h>
#include <sasCore/session.h>

#include <mutex>

namespace SAS {

	struct LoopbackConnection_priv
	{
        LoopbackConnection_priv(Application * app_, Module * module_, const std::string & invoker_name_) :
            app(app_), module(module_), invoker_name(invoker_name_), session_id(0)
		{ }

        Application * app;
		Module * module;
		std::string invoker_name;

		std::mutex session_id_mut;
		SessionID session_id;
	};

    LoopbackConnection::LoopbackConnection(Application * app, Module * module, const std::string & invoker_name) :
			Connection(),
            priv(new LoopbackConnection_priv(app, module, invoker_name))
    { }

	LoopbackConnection::~LoopbackConnection()
	{
		delete priv;
	}

	Invoker::Status LoopbackConnection::invoke(const std::vector<char> & input, std::vector<char> & output, ErrorCollector & ec)
	{
        return priv->app->callIfEnabled<Invoker::Status>([&]() {
            SAS::Session * sess;
            {
                std::unique_lock<std::mutex> __locker(priv->session_id_mut);
                if (!(sess = priv->module->getSession(priv->session_id, ec)))
                    return Invoker::Status::Error;
                priv->session_id = sess->id();
            }
            auto ret = sess->invoke(priv->invoker_name, input, output, ec);
            sess->unlock();
            return ret;
        }, Invoker::Status::Error);
	}

	bool LoopbackConnection::getSession(ErrorCollector & ec)
	{
        return priv->app->callIfEnabled<bool>([&]() {
            std::unique_lock<std::mutex> __locker(priv->session_id_mut);
            Session * sess;
            if (!(sess = priv->module->getSession(priv->session_id, ec)))
                return false;
            priv->session_id = sess->id();
            sess->unlock();
            return true;
        }, false);
    }


	struct LoopbackConnector_priv
	{
        LoopbackConnector_priv(Application * app, const std::string & name) :
            app(app), name(name)
        { }

        Application * app;
		std::string name;
	};

    LoopbackConnector::LoopbackConnector(Application * app, const std::string & name) : priv(new LoopbackConnector_priv(app, name))
    { }

	LoopbackConnector::~LoopbackConnector()
	{
		delete priv;
	}

	std::string LoopbackConnector::name() const
	{
		return priv->name;
	}

	bool LoopbackConnector::connect(ErrorCollector &)
	{
		//nothing to do
		return true;
	}

	bool LoopbackConnector::getModuleInfo(const std::string & module_name, std::string & description, std::string & version, ErrorCollector & ec)
	{
        return priv->app->callIfEnabled<bool>([&]() {
            auto mod = priv->app->objectRegistry()->getObject<SAS::Module>(SAS_OBJECT_TYPE__MODULE, module_name, ec);
            if(!mod)
                return false;
            description = mod->description();
            version = mod->version();
            return true;
        }, false);
	}

	Connection * LoopbackConnector::createConnection(const std::string & module_name, const std::string & invoker_name, ErrorCollector & ec)
	{
		auto mod = priv->app->objectRegistry()->getObject<SAS::Module>(SAS_OBJECT_TYPE__MODULE, module_name, ec);
		if(!mod)
			return nullptr;
        return new LoopbackConnection(priv->app, mod, invoker_name);
	}

}
