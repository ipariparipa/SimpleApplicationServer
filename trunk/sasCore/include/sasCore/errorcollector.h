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

#ifndef INCLUDE_SASCORE_ERRORCOLLECTOR_H_
#define INCLUDE_SASCORE_ERRORCOLLECTOR_H_

#include <string>
#include <functional>
#include "basictypes.h"

namespace SAS
{

class ErrorCollector
{
public:
	virtual inline ~ErrorCollector() { }

	virtual std::string add(long errorCode, const std::string & errorText) = 0;
};

class SimpleErrorCollector : public ErrorCollector
{
public:
	SimpleErrorCollector(std::function<void(long errorCode, const std::string & errorText)> fnct);
	virtual inline ~SimpleErrorCollector() { }
	virtual std::string add(long errorCode, const std::string & errorText) final;

private:
	std::function<void(long errorCode, const std::string & errorText)> _fnct;
};

}

#endif /* INCLUDE_SASCORE_ERRORCOLLECTOR_H_ */
