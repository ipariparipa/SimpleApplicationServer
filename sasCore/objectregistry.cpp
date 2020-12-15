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
#include "include/sasCore/errorcodes.h"

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
    std::recursive_mutex mut;
	struct TypeReg
	{
		std::mutex mut;
		std::map<std::string /*name*/, Object*> reg;
	};

	std::map<std::string /*type*/, std::shared_ptr<TypeReg>> reg;

    std::recursive_mutex lst_mut;
    std::list<std::pair<std::pair<std::string, std::string>, Object*>> lst;

	TypeReg * setTypeReg(const std::string & type)
	{
        std::unique_lock<std::recursive_mutex> __locker(mut);
		if(reg.count(type))
			return reg[type].get();
		auto r = std::make_shared<TypeReg>();
		reg[type] = r;
		return r.get();
	}

	TypeReg * getTypeReg(const std::string & type)
	{
        std::unique_lock<std::recursive_mutex> __locker(mut);
		return reg.count(type) ? reg.at(type).get() : nullptr;
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
					if(tr->reg[o->name()] != o)
					{
						auto err = ec.add(SAS_CORE__ERROR__OBJECT_REGISTRY__ALREADY_REGISTERED, "another object is already registered with the same identifier: '" + o->type() + "/" + o->name() + "'");
						SAS_LOG_ERROR(logger, err);
						has_error = true;
					}
					else
						SAS_LOG_INFO(logger, "object '"+o->type()+"/"+o->name()+"' is already registered");
				}
				else
				{
					tr->reg[o->name()] = o;
                    {
                        std::unique_lock<std::recursive_mutex> __locker(lst_mut);
                        this->lst.push_front(std::make_pair(std::make_pair(o->type(), o->name()), o));
                    }
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
{
    clear();
    delete priv;
}

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
		if(tr->reg[obj->name()] != obj)
		{
			auto err = ec.add(SAS_CORE__ERROR__OBJECT_REGISTRY__ALREADY_REGISTERED, "another object is already registered with the same identifier: '" + obj->type() + "/" + obj->name() + "'");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}
		else
			SAS_LOG_INFO(priv->logger, "object is already registered: '"+obj->type()+"/"+obj->name()+"'");
	}
	else
	{
		tr->reg[obj->name()] = obj;
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

void ObjectRegistry::destroyObject(const std::string & type, const std::string & name)
{
    SAS_LOG_NDC();
    auto tr = priv->getTypeReg(type);
    if(!tr)
    {
        SAS_LOG_WARN(priv->logger, "type is not found in object registry: '" + type + "'");
        return;
    }

    std::unique_lock<std::mutex> __locker(tr->mut);
    auto it = tr->reg.find(name);
    if(it == tr->reg.end())
    {
        SAS_LOG_WARN(priv->logger, "object is not found in registry: '" + type + "/" + name + "'");
        return;
    }

    SAS_LOG_TRACE(priv->logger, std::string("object is found in registry: '")+type+"/"+name+"'");
    delete it->second;
    tr->reg.erase(it);
}

Object * ObjectRegistry::getObject(const std::string & type, const std::string & name, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	auto tr = priv->getTypeReg(type);
	if(!tr)
	{
		auto err = ec.add(SAS_CORE__ERROR__OBJECT_REGISTRY__TYPE_NOT_FOUND, "type is not found in object registry: '" + type + "'");
		SAS_LOG_ERROR(priv->logger, err);
		return nullptr;
	}
	std::unique_lock<std::mutex> __locker(tr->mut);
    auto it = tr->reg.find(name);
    if(it == tr->reg.end())
    {
		auto err = ec.add(SAS_CORE__ERROR__OBJECT_REGISTRY__OBJECT_NOT_FOUND, "object is not found in registry: '" + type + "/" + name + "'");
		SAS_LOG_ERROR(priv->logger, err);
		return nullptr;
	}
	SAS_LOG_TRACE(priv->logger, std::string("object is found in registry: '")+type+"/"+name+"'");
    return it->second;
}

std::vector<Object *> ObjectRegistry::getObjects(const std::string & type, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	std::vector<Object *> ret;

	SAS_LOG_VAR(priv->logger, type);
	auto tr = priv->getTypeReg(type);
	if(!tr)
	{
		auto err = ec.add(SAS_CORE__ERROR__OBJECT_REGISTRY__TYPE_NOT_FOUND, std::string("type is not found in object registry: '") + type + "'");
		SAS_LOG_ERROR(priv->logger, err);
		return ret;
	}
	std::unique_lock<std::mutex> __locker(tr->mut);
	if(!tr->reg.size())
	{
		auto err = ec.add(SAS_CORE__ERROR__OBJECT_REGISTRY__OBJECT_NOT_FOUND, std::string("no objects are found in object registry for type: '") + type + "'");
		SAS_LOG_ERROR(priv->logger, err);
		return ret;
	}
	SAS_LOG_TRACE(priv->logger, std::to_string(tr->reg.size()) + std::string(" object(s) are found in registry"));
	ret.resize(tr->reg.size());
	size_t i(0);
	for(auto & o : tr->reg)
		ret[i++] = o.second;
	return ret;
}

void ObjectRegistry::clear()
{
    std::unique_lock<std::recursive_mutex> __locker(priv->lst_mut);

    for(auto e : priv->lst)
    {
        auto name = e.second->name();
        SAS_LOG_INFO(priv->logger, "deinit object '"+name+"'...");
        e.second->deinit();
        SAS_LOG_INFO(priv->logger, "deinit object '"+name+"'... ..done");
    }

    SAS_LOG_INFO(priv->logger, "destroying objects...");
    for(auto e : priv->lst)
        destroyObject(e.first.first, e.first.second);
    SAS_LOG_INFO(priv->logger, "destroying objects... ..done");

    priv->lst.clear();
}

}
