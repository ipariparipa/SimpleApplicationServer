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

#pragma once
#pragma warning( push )
#pragma warning( disable: 4461 )

#include "SASObject.h"
#include <sasCore/connector.h>

namespace SAS {
	namespace Client{

		ref class SASBinData;

		interface class ISASErrorCollector;

		public interface class ISASInvoker
		{
		public:
			enum class Status
			{
				OK, Error, FatalError, NotImplemented
			};

			virtual Status Invoke(SASBinData ^ input, [System::Runtime::InteropServices::OutAttribute] SASBinData ^% output, ISASErrorCollector ^ ec) abstract;

		};

		public interface class ISASConnection : public ISASInvoker
		{
		public:
			virtual bool GetSession(ISASErrorCollector ^ ec) abstract;
		};


		struct SASConnectionObj_priv;
		ref class SASConnectionObj : public ISASConnection
		{
		public:
			SASConnectionObj(Connection * obj);
			!SASConnectionObj();
			virtual ISASInvoker::Status Invoke(SASBinData ^ input, [System::Runtime::InteropServices::OutAttribute] SASBinData ^% output, ISASErrorCollector ^ ec);
			virtual bool GetSession(ISASErrorCollector ^ ec);
		private:
			SASConnectionObj_priv * priv;
		};


		public interface class ISASConnector : public ISASObject
		{
			virtual bool Connect(ISASErrorCollector ^ ec) abstract;

			virtual bool GetModuleInfo(System::String ^ module_name, [System::Runtime::InteropServices::OutAttribute] System::String ^% description, [System::Runtime::InteropServices::OutAttribute] System::String ^% version, ISASErrorCollector ^ ec) abstract;

			virtual ISASConnection ^ CreateConnection(System::String ^ module_name, System::String ^ invoker_name, ISASErrorCollector ^ ec) abstract;
		};


		struct SASConnectorObj_priv;
		ref class SASConnectorObj : public ISASConnector
		{
		public:
			SASConnectorObj(Connector * obj);
			!SASConnectorObj();

			property System::String ^ Type { virtual System::String ^ get(); }
			property System::String ^ Name { virtual System::String ^ get(); }

			virtual bool Connect(ISASErrorCollector ^ ec);

			virtual bool GetModuleInfo(System::String ^ module_name, [System::Runtime::InteropServices::OutAttribute] System::String ^% description, [System::Runtime::InteropServices::OutAttribute] System::String ^% version, ISASErrorCollector ^ ec);

			virtual ISASConnection ^ CreateConnection(System::String ^ module_name, System::String ^ invoker_name, ISASErrorCollector ^ ec);
		private:
			SASConnectorObj_priv * priv;
		};
	}
}

#pragma warning( pop ) 
