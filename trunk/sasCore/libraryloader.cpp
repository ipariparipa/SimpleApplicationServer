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

#include "include/sasCore/libraryloader.h"

#include <assert.h>
#if SAS_OS == SAS_OS_LINUX
#  include <dlfcn.h>
#elif SAS_OS == SAS_OS_WINDOWS
#include <Windows.h>
#else
#  error to be implemented
#endif
#include "include/sasCore/errorcollector.h"

#include "include/sasCore/tools.h"

namespace SAS {

	struct LibraryLoader_priv
	{
		LibraryLoader_priv() : handle(nullptr)
		{ }
#if SAS_OS == SAS_OS_LINUX
		void * handle;
#elif SAS_OS == SAS_OS_WINDOWS
		HMODULE handle;
#else
#  error to be implemented
#endif
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
#if SAS_OS == SAS_OS_LINUX
		if(!(priv->handle = dlopen(filename.c_str(), RTLD_LAZY)))
		{
			ec.add(-1, "could not open library: '" + filename + "': " + dlerror());
			return false;
		}
#elif SAS_OS == SAS_OS_WINDOWS
		if (!(priv->handle = LoadLibrary(filename.c_str())))
		{
			ec.add(-1, "could not open library: '" + filename + "': " + win_getLastErrorMessage());
			return false;
		}
#else
#  error to be implemented
#endif
		priv->filename = filename;
		return true;
	}

	void LibraryLoader::unload()
	{
		if(!priv->handle)
			return;
		if (priv->handle)
#if SAS_OS == SAS_OS_LINUX
			dlclose(priv->handle);
#elif SAS_OS == SAS_OS_WINDOWS
			if (!FreeLibrary(priv->handle))
			{
				//TODO
			}
#else
#  error to be implemented
#endif
		priv->handle = nullptr;
		priv->filename.clear();
	}

	void * LibraryLoader::getProcedure(const char * name, ErrorCollector & ec)
	{
		assert(priv->handle);
		void * ret;
#if SAS_OS == SAS_OS_LINUX
		if (!(ret = dlsym(priv->handle, name)))
		{
			ec.add(-1, "could not find procedure '" + std::string(name) + "' in library '" + priv->filename + "': " + dlerror());
			return nullptr;
		}
#elif SAS_OS == SAS_OS_WINDOWS
		if (!(ret = GetProcAddress(priv->handle, name)))
		{
			ec.add(-1, "could not find procedure '" + std::string(name) + "' in library '" + priv->filename + "': " + win_getLastErrorMessage());
			return nullptr;
		}
#else
#  error to be implemented
#endif
		return ret;
	}

}
