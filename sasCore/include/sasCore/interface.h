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

#ifndef INCLUDE_SASCORE_INTERFACE_H_
#define INCLUDE_SASCORE_INTERFACE_H_

#include "defines.h"

#include <string>

namespace SAS {

class ErrorCollector;

class SAS_CORE__CLASS Interface
{
	SAS_COPY_PROTECTOR(Interface)
protected:
	inline Interface() { }
public:
	enum class Status
	{
		Unexpected,
		CannotStart,
		Started,
		Stopped,
		Crashed,
		CannotStop,
		Ended
	};

	virtual inline ~Interface() { }

	virtual std::string name() const = 0;

	virtual Status run(ErrorCollector & ec) = 0;

	virtual Status shutdown(ErrorCollector & ec) = 0;
};

}

#endif /* INCLUDE_SASCORE_INTERFACE_H_ */
