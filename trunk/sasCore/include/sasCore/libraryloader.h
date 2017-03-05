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

#ifndef INCLUDE_SAS_CORE_MODULE_H_
#define INCLUDE_SAS_CORE_MODULE_H_

#include <string>
#include "defines.h"

namespace SAS {

class ErrorCollector;

struct LibraryLoader_priv;
class LibraryLoader
{
	SAS_COPY_PROTECTOR(LibraryLoader)
public:
	LibraryLoader();
	~LibraryLoader();
	bool load(const std::string & filename, ErrorCollector & ec);
	void unload();
	void * getProcedure(const char * name, ErrorCollector & ec);

private:
	LibraryLoader_priv * priv;
};


}


#endif /* INCLUDE_SAS_CORE_MODULE_H_ */
