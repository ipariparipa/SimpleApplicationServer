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

#ifndef INCLUDE_SASCORE_OBJECT_H_
#define INCLUDE_SASCORE_OBJECT_H_

#include "defines.h"

#include <string>

namespace SAS {

class SAS_CORE__CLASS Object
{
protected:
	inline Object() { }
public:
	virtual inline ~Object() { }

	virtual std::string type() const = 0;
	virtual std::string name() const = 0;

    virtual inline void deinit() { }
};

}

#endif /* INCLUDE_SASCORE_OBJECT_H_ */
