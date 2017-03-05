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

#include "include/sasCore/objectregistry.h"
#include "include/sasCore/object.h"
#include "include/sasCore/logging.h"
#include "include/sasCore/errorcollector.h"

#include <mutex>
#include <map>
#include <assert.h>
#include <memory>

namespace SAS {

struct ObjectRegistry_priv
{
	ObjectRegistry_priv() : logger(Logging::getLogger("SAS.ObjectRegistry"))
	{ }

	Logging::LoggerPtr logger;
	std::mutex mut;
	struct TypeReg
	{
		std::mutex mut;
		std::map<std::string /*name*/, std::unique_ptr<Object>> reg;
	};

	std::map<std::string /*type*/, TypeReg> reg;

	TypeReg * setTypeReg(const std::string & type)
	{
		std::unique_lock<std::mutex> __locker(mut);
		return &reg[type];
	}

	TypeReg * getTypeReg(const std::string & type)
	{
		std::unique_lock<std::mutex> __locker(mut);
		return reg.count(type) ? &reg[type] : nullptr;
	}

	bool registerObjects(std::map<std::string /*type*/, std::list<Object *>> obj, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		bool has_error(false);
		for(auto & lst : obj)
		{
			ObjectRegistry_priv::TypeReg * tr = setTypeReg(lst.first);
			assert(tr);
			std::unique_lock<std::mutex> __locker(tr->mut);
			for(auto & o : lst.second)
			{
				SAS_LOG_ASSERT(logger, o, "object must not be NULL");
				if(tr->reg.count(o->name()))
				{
					SAS_LOG_VAR(logger, o->type());
					SAS_LOG_VAR(logger, o->name());
					if(tr->reg[o->name()].get() != o)
					{
						auto err = ec.add(-1, "another object is already registered with the same identifier: '"+o->type()+"/"+o->name()+"'");
						SAS_LOG_ERROR(logger, err);
						has_error = true;
					}
					else
						SAS_LOG_INFO(logger, "object '"+o->type()+"/"+o->name()+"' is already registered");
				}
				else
				{
					tr->reg[o->name()].reset(o);
					SAS_LOG_DEBUG(logger, "object '"+o->type()+"/"+o->name()+"' has been registered");
				}
			}
		}
		return !has_error;
	}

};

ObjectRegistry::ObjectRegistry() : priv(new ObjectRegistry_priv)
{ }

ObjectRegistry::~ObjectRegistry()
{ delete priv; }

bool ObjectRegistry::registerObject(Object * obj, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	assert(obj);
	ObjectRegistry_priv::TypeReg * tr = priv->setTypeReg(obj->type());
	assert(tr);
	std::unique_lock<std::mutex> __locker(tr->mut);
	if(tr->reg.count(obj->name()))
	{
		SAS_LOG_VAR(priv->logger, obj->type());
		SAS_LOG_VAR(priv->logger, obj->name());
		if(tr->reg[obj->name()].get() != obj)
		{
			auto err = ec.add(-1, "another object is already registered with the same identifier: '"+obj->type()+"/"+obj->name()+"'");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}
		else
			SAS_LOG_INFO(priv->logger, "object is already registered: '"+obj->type()+"/"+obj->name()+"'");
	}
	else
	{
		tr->reg[obj->name()].reset(obj);
		SAS_LOG_DEBUG(priv->logger, "object '"+obj->type()+"/"+obj->name()+"' has been registered");
	}
	return true;
}

bool ObjectRegistry::registerObjects(std::vector<Object *> obj, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	std::map<std::string /*type*/, std::list<Object *>> tmp;
	for(auto & o : obj)
		tmp[o->type()].push_back(o);
	return priv->registerObjects(tmp, ec);
}

Object * ObjectRegistry::getObject(const std::string & type, const std::string & name, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	auto tr = priv->getTypeReg(type);
	if(!tr)
	{
		auto err = ec.add(-1, "type is not found in object registry: '"+type+"'");
		SAS_LOG_ERROR(priv->logger, err);
		return nullptr;
	}
	std::unique_lock<std::mutex> __locker(tr->mut);
	if(!tr->reg.count(name))
	{
		auto err = ec.add(-1, "object is not found in registry: '"+type+"/"+name+"'");
		SAS_LOG_ERROR(priv->logger, err);
		return nullptr;
	}
	SAS_LOG_TRACE(priv->logger, std::string("object is found in registry: '")+type+"/"+name+"'");
	return tr->reg[name].get();
}

std::vector<Object *> ObjectRegistry::getObjects(const std::string & type, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	std::vector<Object *> ret;

	SAS_LOG_VAR(priv->logger, type);
	auto tr = priv->getTypeReg(type);
	if(!tr)
	{
		auto err = ec.add(-1, std::string("type is not found in object registry: '")+type+"'");
		SAS_LOG_ERROR(priv->logger, err);
		return ret;
	}
	std::unique_lock<std::mutex> __locker(tr->mut);
	if(!tr->reg.size())
	{
		auto err = ec.add(-1, std::string("no objects are found in object registry for type: '")+type+"'");
		SAS_LOG_ERROR(priv->logger, err);
		return ret;
	}
	SAS_LOG_TRACE(priv->logger, std::to_string(tr->reg.size()) + std::string(" object(s) are found in registry"));
	ret.resize(tr->reg.size());
	size_t i(0);
	for(auto & o : tr->reg)
		ret[i++] = o.second.get();
	return ret;
}


}



