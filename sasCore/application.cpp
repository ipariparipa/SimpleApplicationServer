/*
    This file is part of sasCore.

    sasCore is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasCore is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasCore.  If not, see <http://www.gnu.org/licenses/>
 */

#include "include/sasCore/application.h"
#include "include/sasCore/interfacemanager.h"
#include "include/sasCore/objectregistry.h"
#include "include/sasCore/thread.h"
#include "include/sasCore/watchdog.h"
#include "include/sasCore/logging.h"
#include "include/sasCore/componentloader.h"
#include "include/sasCore/configreader.h"
#include "include/sasCore/component.h"
#include "include/sasCore/errorcodes.h"
#include "include/sasCore/threadpool.h"

#include <list>
#include <memory>
#include <condition_variable>

#include <stdlib.h>
#include <time.h>

namespace SAS {

struct Application_priv
{
    Application_priv() :
        logger(Logging::getLogger("SAS.Application")),
        threadPool("SAS.Application.ThreadPool"),
        argc(0),
        argv(nullptr)
    {
		srand((unsigned int)time(0));
	}

	ObjectRegistry objectRegistry;
	std::vector<ComponentLoader*> componentLoaders;
	Logging::LoggerPtr logger;
    SimpleThreadPool threadPool;

	int argc;
	char ** argv;

    bool enabled = false;

    #ifdef SAS_APP_SMART_LOCKING
        std::mutex lock_mut;
        std::condition_variable lock_cv;
        int lock_counter = 0;
#else
        std::recursive_mutex lock_mut;
#endif
};

Application::Application() : priv(new Application_priv)
{ }

Application::Application(int argc, char **argv) : priv(new Application_priv)
{
	priv->argc = argc;
	priv->argv = argv;
}

Application::~Application()
{
    deinit();
    delete priv;
}

int Application::argc() const
{
	return priv->argc;
}

char ** Application::argv() const
{
	return priv->argv;
}

ObjectRegistry * Application::objectRegistry() const
{
	return &priv->objectRegistry;
}

Logging::LoggerPtr Application::logger()
{
	return priv->logger;
}

bool Application::init(ErrorCollector & ec)
{
	SAS_LOG_NDC();

    bool has_error(false);

    {
        std::unique_lock<Application> __locker(*this);

        priv->enabled = true;

        SAS_LOG_INFO(logger(), "activating components");
        std::vector<std::string> comp_paths;
        if (configReader()->getStringListEntry("SAS/COMPONENTS", comp_paths, ec))
        {
            if(!comp_paths.size())
            {
                auto err = ec.add(SAS_CORE__ERROR__APPLICATION__NO_COMPONENTS, "components are not set");
                SAS_LOG_ERROR(logger(), err);
                return false;
            }

            priv->componentLoaders.resize(comp_paths.size());

            SAS_LOG_INFO(logger(), "loading component libraries...");
            for(size_t i = 0, l = comp_paths.size(); i < l; ++i)
            {
                SAS_LOG_VAR(logger(), comp_paths[i]);
                auto cl = new ComponentLoader(comp_paths[i]);
                if(!cl->load(ec))
                {
                    delete cl;
                    has_error = true;
                }
                else
                    priv->componentLoaders[i] = cl;
            }
            if(has_error)
            {
                SAS_LOG_ERROR(logger(), "loading component libraries... ..error");
                deinit();
                return false;
            }
            SAS_LOG_INFO(logger(), "loading component libraries... ..done");

            SAS_LOG_INFO(logger(), "initializing components...");
            for(auto cl : priv->componentLoaders)
            {
                auto comp = cl->component();
                SAS_LOG_ASSERT(logger(), comp, "component is not available");
                SAS_LOG_INFO(logger(), "component: '"+comp->name()+"'; version: '"+comp->version()+"'; vendor: '"+comp->vendor()+"'");
                std::stringstream ss;
                bool first = true;
                for(auto & v : comp->customInfo())
                {
                    if(first)
                        first = false;
                    else
                        ss << "; ";
                    ss << v.first << ": '" << v.second << "'";
                }
                SAS_LOG_INFO(logger(), ss.str());
                SAS_LOG_DEBUG(logger(), "initializing component '"+comp->name()+"'...");
                if(!comp->init(this, ec))
                {
                    SAS_LOG_ERROR(logger(), "initializing component '"+comp->name()+"'... ..error");
                    deinit();
                    return false;
                }
                SAS_LOG_INFO(logger(), "initializing component '"+comp->name()+"'... ..done");
            }
            SAS_LOG_INFO(logger(), "initializing components... ..done");
        }
        else
            SAS_LOG_WARN(logger(), "no components are set");
    }

    if(has_error)
    {
        deinit();
        return false;
    }

    return true;
}

void Application::deinit()
{
	SAS_LOG_NDC();

    //set app to disable
    {
    #ifdef SAS_APP_SMART_LOCKING
        std::unique_lock<std::mutex> __locker(priv->lock_mut);
        priv->lock_cv.wait(__locker, [&]() { return priv->lock_counter == 0; });
    #else
        std::unique_lock<std::recursive_mutex> __locker(priv->lock_mut);
    #endif

        priv->enabled = false;
    }

    auto im = interfaceManager();
	if(im)
		im->terminate();

    priv->objectRegistry.clear();

    for(auto cl : priv->componentLoaders)
		if(cl)
			delete cl;
	priv->componentLoaders.clear();
}

//virtual
ThreadPool * Application::threadPool()
{
    return &priv->threadPool;
}

void Application::lock()
{
#ifdef SAS_APP_SMART_LOCKING
    std::unique_lock<std::mutex> __locker(priv->lock_mut);
    ++priv->lock_counter;
#else
    priv->lock_mut.lock();
#endif
}

void Application::unlock()
{
#ifdef SAS_APP_SMART_LOCKING
    std::unique_lock<std::mutex> __locker(priv->lock_mut);
    if(--priv->lock_counter < 0)
        priv->lock_counter = 0;
#else
    priv->lock_mut.unlock();
#endif
}

bool Application::isEnabled()
{
    return priv->enabled;
}

}
