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

#include "SASConnector.h"
#include "SASErrorCollector.h"
#include "SASBinData.h"

#include "macros.h"

#include <memory>

namespace SAS {
	namespace Client {


		struct SASConnectionObj_priv
		{
			SASConnectionObj_priv(Connection * obj_) : obj(obj_)
			{ }

			std::unique_ptr<Connection> obj;
		};

		SASConnectionObj::SASConnectionObj(Connection * obj) : priv(new SASConnectionObj_priv(obj))
		{ }

		SASConnectionObj::!SASConnectionObj()
		{
			delete priv;
		}

		ISASInvoker::Status SASConnectionObj::Invoke(SASBinData ^ input, [System::Runtime::InteropServices::OutAttribute] SASBinData ^% output, ISASErrorCollector ^ ec)
		{

			output = gcnew SASBinData();

			auto tmp_ret = priv->obj->invoke(input->data(), output->data(), WErrorCollector(ec));
			if (tmp_ret != Invoker::Status::OK)
				output = nullptr;
			
			return (ISASInvoker::Status)tmp_ret;
		}


		struct SASConnectorObj_priv
		{
			SASConnectorObj_priv(Connector * obj_) : obj(obj_)
			{ }

			Connector * obj;
		};

		SASConnectorObj::SASConnectorObj(Connector * obj) : priv(new SASConnectorObj_priv(obj))
		{ }

		SASConnectorObj::!SASConnectorObj()
		{
			delete priv;
		}

		//property 
		System::String ^ SASConnectorObj::Type::get()
		{
			return TO_MSTR(priv->obj->type());
		}

		//property 
		System::String ^ SASConnectorObj::Name::get()
		{
			return TO_MSTR(priv->obj->name());
		}

		bool SASConnectorObj::Connect(ISASErrorCollector ^ ec)
		{
			return priv->obj->connect(WErrorCollector(ec));
		}

		ISASConnection ^ SASConnectorObj::CreateConnection(System::String ^ module_name, System::String ^ invoker_name, ISASErrorCollector ^ ec)
		{
			auto tmp = priv->obj->createConnection(TO_STR(module_name), TO_STR(invoker_name), WErrorCollector(ec));
			if (!tmp)
				return nullptr;
			return gcnew SASConnectionObj(tmp);
		}

	}
}
