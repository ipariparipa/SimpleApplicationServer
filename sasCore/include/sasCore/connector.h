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

#ifndef INCLUDE_SASCORE_CONNECTOR_H_
#define INCLUDE_SASCORE_CONNECTOR_H_

#include "object.h"
#include "invoker.h"

#define SAS_OBJECT_TYPE__CONNECTOR "connector"

namespace SAS {

class ErrorCollector;

class SAS_CORE__CLASS Connection : public Invoker
{
	SAS_COPY_PROTECTOR(Connection)
public:
	inline Connection() : Invoker() { }
	virtual inline ~Connection() { }

	virtual bool getSession(ErrorCollector & ec) =  0;
};


class Connector : public Object
{
	SAS_COPY_PROTECTOR(Connector)
public:
	inline Connector() { }
	virtual inline ~Connector() { }

	virtual inline std::string type() const final
			{ return SAS_OBJECT_TYPE__CONNECTOR; }

	virtual bool connect(ErrorCollector & ec) = 0;

	virtual bool getModuleInfo(const std::string & module_name, std::string & description, std::string & version, ErrorCollector & ec) = 0;

	virtual Connection * createConnection(const std::string & module_name, const std::string & invoker_name, ErrorCollector & ec) = 0;
};

}

#endif /* INCLUDE_SASCORE_CONNECTOR_H_ */
