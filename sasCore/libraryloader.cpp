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

#include "include/sasCore/libraryloader.h"

#include <assert.h>
#include <dlfcn.h>
#include "include/sasCore/errorcollector.h"

namespace SAS {

	struct LibraryLoader_priv
	{
		LibraryLoader_priv() : handle(nullptr)
		{ }
		void * handle;
		std::string filename;
	};


	LibraryLoader::LibraryLoader() : priv(new LibraryLoader_priv)
	{ }

	LibraryLoader::~LibraryLoader()
	{
		unload();
		delete priv;
	}

	bool LibraryLoader::load(const std::string & filename, ErrorCollector & ec)
	{
		assert(!priv->handle);
		assert(filename.length());
		if(!(priv->handle = dlopen(filename.c_str(), RTLD_LAZY)))
		{
			ec.add(-1, "could not open library: '" + filename + "': " + dlerror());
			return false;
		}
		priv->filename = filename;
		return true;
	}

	void LibraryLoader::unload()
	{
		if(!priv->handle)
			return;
		dlclose(priv->handle);
		priv->handle = nullptr;
		priv->filename.clear();
	}

	void * LibraryLoader::getProcedure(const char * name, ErrorCollector & ec)
	{
		assert(priv->handle);
		void * ret;
		if(!(ret = dlsym(priv->handle, name)))
		{
			ec.add(-1, "could not find procedure '" + std::string(name) + "' in library '" + priv->filename + "': " + dlerror());
			return nullptr;
		}
		return ret;
	}

}
