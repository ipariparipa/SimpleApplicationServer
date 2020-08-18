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

#ifndef INCLUDE_SASCORE_MODULE_H_
#define INCLUDE_SASCORE_MODULE_H_

#include "object.h"
#include "sessionmanager.h"

#include <string>

#define SAS_OBJECT_TYPE__MODULE "module"

namespace SAS
{

class SAS_CORE__CLASS Module : public SessionManager, public Object
{
public:
    Module(Application * app);
	virtual ~Module();

	virtual inline std::string type() const final
			{ return SAS_OBJECT_TYPE__MODULE; }

	virtual inline std::string description() const { return std::string(); }
	virtual inline std::string version() const { return std::string(); }
};

}

#endif /* INCLUDE_SASCORE_MODULE_H_ */
