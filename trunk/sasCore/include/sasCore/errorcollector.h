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

class SAS_CORE__CLASS ErrorCollector
{
public:
	virtual inline ~ErrorCollector() { }

	std::string add(long errorCode, const std::string & errorText);
	static std::string toString(long errorCode, const std::string & errorText);
protected:
	virtual void append(long errorCode, const std::string & errorText) = 0;
};

struct NullEC : public ErrorCollector
{
	virtual ~NullEC() { }
	virtual void append(long errorCode, const std::string & errorText) override { }
};

struct SimpleErrorCollector_priv;

class SAS_CORE__CLASS SimpleErrorCollector : public ErrorCollector
{
public:
	SimpleErrorCollector(std::function<void(long errorCode, const std::string & errorText)> fnct);
	virtual ~SimpleErrorCollector();

protected:
	virtual void append(long errorCode, const std::string & errorText) final;
private:
	SimpleErrorCollector_priv * priv;
};

}

#endif /* INCLUDE_SASCORE_ERRORCOLLECTOR_H_ */
