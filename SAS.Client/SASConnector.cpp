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

#pragma warning( push )
#pragma warning( disable: 4461 )

#include "SASConnector.h"
#include "SASErrorCollector.h"
#include "SASBinData.h"

#include "macros.h"

#include <memory>

namespace SAS {
	namespace Client {

		struct SASConnectionObj_um_priv
		{
			SASConnectionObj_um_priv(Connection * obj_) : obj(obj_)
			{ }

			std::unique_ptr<Connection> obj;
		};

		ref struct SASConnectionObj_priv
		{
			SASConnectionObj_priv(Connection * obj_) : um(new SASConnectionObj_um_priv(obj_))
			{ }

			!SASConnectionObj_priv()
			{
				delete um;
			}

			SASConnectionObj_um_priv * um;
		};

		SASConnectionObj::SASConnectionObj(Connection * obj) : priv(gcnew SASConnectionObj_priv(obj))
		{ }

		ISASInvoker::Status SASConnectionObj::Invoke(SASBinData ^ input, [System::Runtime::InteropServices::OutAttribute] SASBinData ^% output, ISASErrorCollector ^ ec)
		{
			output = gcnew SASBinData();

			auto tmp_ret = priv->um->obj->invoke(input->data(), output->data(), WErrorCollector(ec));
			if (tmp_ret != Invoker::Status::OK)
				output = nullptr;
			
			System::GC::KeepAlive(input);
			System::GC::KeepAlive(ec);

			return (ISASInvoker::Status)tmp_ret;
		}

		bool SASConnectionObj::GetSession(ISASErrorCollector ^ ec)
		{
			return priv->um->obj->getSession(WErrorCollector(ec));
		}


		ref struct SASConnectorObj_priv
		{
			SASConnectorObj_priv(Connector * obj_) : obj(obj_)
			{ }

			Connector * obj;
		};

		SASConnectorObj::SASConnectorObj(Connector * obj) : priv(gcnew SASConnectorObj_priv(obj))
		{ }

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

		bool SASConnectorObj::GetModuleInfo(System::String ^ module_name, [System::Runtime::InteropServices::OutAttribute] System::String ^% description, [System::Runtime::InteropServices::OutAttribute] System::String ^% version, ISASErrorCollector ^ ec)
		{
			std::string _description, _version;
			if (!priv->obj->getModuleInfo(TO_STR(module_name), _description, _version, WErrorCollector(ec)))
				return false;
			description = TO_MSTR(_description);
			version = TO_MSTR(_version);
			return true;
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

#pragma warning( pop )
