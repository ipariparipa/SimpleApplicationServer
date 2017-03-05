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

#ifndef INCLUDE_SASCORE_OBJECTREGISTRY_H_
#define INCLUDE_SASCORE_OBJECTREGISTRY_H_

#include "defines.h"
#include <string>
#include <vector>
#include <list>

namespace SAS {

class Object;
class ErrorCollector;

struct ObjectRegistry_priv;

class SAS_CORE__CLASS ObjectRegistry
{
	SAS_COPY_PROTECTOR(ObjectRegistry)
public:
	ObjectRegistry();
	~ObjectRegistry();

	bool registerObject(Object * obj, ErrorCollector & ec);

	bool registerObjects(std::vector<Object *> obj, ErrorCollector & ec);

	Object * getObject(const std::string & type, const std::string & name, ErrorCollector & ec);

	template<class Object_T>
	Object_T * getObject(const std::string & type, const std::string & name, ErrorCollector & ec)
	{
		return dynamic_cast<Object_T*>(getObject(type, name, ec));
	}

	std::vector<Object *> getObjects(const std::string & type, ErrorCollector & ec);

	template<class Object_T>
	std::list<Object_T*> getObjects(const std::string & type, ErrorCollector & ec)
	{
		std::list<Object_T*> ret;
		for(auto & o : getObjects(type, ec))
		{
			auto _o = dynamic_cast<Object_T*>(o);
			if(_o)
				ret.push_back(_o);
		}
		return ret;
	}
private:
	ObjectRegistry_priv * priv;
};

}

#endif /* INCLUDE_SASCORE_OBJECTREGISTRY_H_ */
