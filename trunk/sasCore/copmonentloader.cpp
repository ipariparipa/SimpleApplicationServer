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
    along with ${project_name}.  If not, see <http://www.gnu.org/licenses/>
 */

#include "assert.h"
#include "include/sasCore/componentloader.h"
#include "include/sasCore/libraryloader.h"
#include "include/sasCore/errorcollector.h"

namespace SAS {

	struct ComponentLoader_priv
	{
		ComponentLoader_priv() : comp(nullptr), attacher(nullptr), detacher(nullptr)
		{ }

		std::string filename;
		Component * comp;
		LibraryLoader loader;

		typedef Component * (*attacher_T)(void);
		typedef void (*detacher_T)(Component * );

		attacher_T attacher;
		detacher_T detacher;
	};

	ComponentLoader::ComponentLoader(const std::string & filename) : priv(new ComponentLoader_priv)
	{
		priv->filename = filename;
	}

	ComponentLoader::~ComponentLoader()
	{
		unload();
		delete priv;
	}

	const std::string & ComponentLoader::filename() const
	{
		return priv->filename;
	}

	bool ComponentLoader::load(ErrorCollector & ec)
	{
		if(!priv->loader.load(priv->filename, ec))
			return false;
		bool has_error(false);
		if(!(priv->attacher = (ComponentLoader_priv::attacher_T)priv->loader.getProcedure("__sas_attach_component", ec)))
			has_error = true;
		if(!(priv->detacher = (ComponentLoader_priv::detacher_T)priv->loader.getProcedure("__sas_detach_component", ec)))
			has_error = true;
		if(has_error)
			return false;

		if(!(priv->comp = priv->attacher()))
		{
			ec.add(-1, "unable to get entrance object of the component '" + priv->filename + "'");
			return false;
		}

		return true;
	}

	void ComponentLoader::unload()
	{
		if(!priv->comp)
			return;
		assert(priv->detacher);

		priv->detacher(priv->comp);
		priv->comp = nullptr;
		priv->loader.unload();
	}

	Component * ComponentLoader::component() const
	{
		return priv->comp;
	}

}




