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

#include <list>
#include <memory>

namespace SAS {

struct Application_priv
{
	Application_priv() : logger(Logging::getLogger("SAS.Application")), argc(0), argv(nullptr)
	{ }

	ObjectRegistry objectRegistry;
	std::vector<ComponentLoader*> componentLoaders;
	Logging::LoggerPtr logger;

	int argc;
	char ** argv;
};

Application::Application() : priv(new Application_priv)
{ }

Application::Application(int argc, char **argv) : priv(new Application_priv)
{
	priv->argc = argc;
	priv->argv = argv;
}

Application::~Application()
{ delete priv; }

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

	SAS_LOG_INFO(logger(), "activating components");
	std::vector<std::string> comp_paths;
	if (!configReader()->getStringListEntry("SAS/COMPONENTS", comp_paths, ec) || !comp_paths.size())
	{
		auto err = ec.add(SAS_CORE__ERROR__APPLICATION__NO_COMPONENTS, "components are not set");
		SAS_LOG_ERROR(logger(), err);
		return false;
	}

	priv->componentLoaders.resize(comp_paths.size());

	bool has_error(false);
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
		deinit();
		return false;
	}

	SAS_LOG_INFO(logger(), "initializing components");
	for(auto cl : priv->componentLoaders)
	{
		if(!cl->component()->init(this, ec))
			has_error = true;
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
	auto im = interfaceManager();
	if(im)
		im->terminate();
	for(auto cl : priv->componentLoaders)
		if(cl)
			delete cl;
	priv->componentLoaders.clear();
}

}
