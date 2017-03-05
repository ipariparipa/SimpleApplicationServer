/*
    This file is part of SAS.Client.

    SAS.Client is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SAS.Client is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with SAS.Client.  If not, see <http://www.gnu.org/licenses/>
 */

#include "SASObjectRegistry.h"
#include "SASErrorCollector.h"
#include "SASObject.h"
#include "SASConnector.h"

#include "macros.h"

#include <sasCore/objectregistry.h>
#include <sasCore/connector.h>

namespace SAS {
	namespace Client {

		struct SASObjectRegistry_priv
		{
			SASObjectRegistry_priv(ObjectRegistry * obj_) : obj(obj_)
			{ }

			ObjectRegistry * obj;
		};
		
		SASObjectRegistry::SASObjectRegistry(ObjectRegistry * obj) : priv(new SASObjectRegistry_priv(obj))
		{ }

		SASObjectRegistry::!SASObjectRegistry()
		{
			delete priv;
		}

		ISASObject ^ SASObjectRegistry::getObject(System::String ^ type, System::String ^ name, ISASErrorCollector ^ ec)
		{
			auto tmp = priv->obj->getObject(TO_STR(type), TO_STR(name), WErrorCollector(ec));
			return tmp ? gcnew SASObjectObj(tmp) : nullptr;
		}

		ISASConnector ^ SASObjectRegistry::GetConnector(System::String ^ name, ISASErrorCollector ^ ec)
		{
			auto tmp = priv->obj->getObject(SAS_OBJECT_TYPE__CONNECTOR, TO_STR(name), WErrorCollector(ec));
			if (!tmp)
				return nullptr;
			if (dynamic_cast<Connector*>(tmp))
				return dynamic_cast<ISASConnector^>(gcnew SASConnectorObj(dynamic_cast<Connector*>(tmp)));

			return dynamic_cast<ISASConnector^>(gcnew SASObjectObj(tmp));
		}

		array<ISASObject^> ^ SASObjectRegistry::GetObjects(System::String ^ type, ISASErrorCollector ^ ec)
		{
			auto tmp = priv->obj->getObjects(TO_STR(type), WErrorCollector(ec));
			auto ret = gcnew array<ISASObject^>(tmp.size());
			int i(0);
			for (auto & o : tmp)
				ret[i] = gcnew SASObjectObj(o);
			return ret;
		}

		array<ISASConnector^> ^ SASObjectRegistry::GetConnectors(ISASErrorCollector ^ ec)
		{
			auto tmp = priv->obj->getObjects<Connector>(SAS_OBJECT_TYPE__CONNECTOR, WErrorCollector(ec));
			auto ret = gcnew array<ISASConnector^>(tmp.size());
			int i(0);
			for (auto & o : tmp)
				ret[i++] = gcnew SASConnectorObj(o);
			return ret;
		}
	}
}