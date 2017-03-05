
#pragma once
#pragma warning( push )
#pragma warning( disable: 4461 )

#include "SASObject.h"
#include <sasCore/connector.h>

namespace SAS {
	namespace Client{

		interface class ISASErrorCollector;

		public interface class ISASInvoker
		{
			enum class Status
			{
				OK, Error, FatalError, NotImplemented
			};

			virtual Status Invoke(array<System::Byte> ^ input, [System::Runtime::InteropServices::OutAttribute] array<System::Byte> ^% output, ISASErrorCollector ^ ec) abstract;

		};

		public interface class ISASConnection : public ISASInvoker
		{

		};


		struct SASConnectionObj_priv;
		ref class SASConnectionObj : public ISASConnection
		{
		public:
			SASConnectionObj(Connection * obj);
			!SASConnectionObj();
			virtual ISASInvoker::Status Invoke(array<System::Byte> ^ input, [System::Runtime::InteropServices::OutAttribute] array<System::Byte> ^% output, ISASErrorCollector ^ ec);
		private:
			SASConnectionObj_priv * priv;
		};


		public interface class ISASConnector : public ISASObject
		{
			virtual bool Connect(ISASErrorCollector ^ ec) abstract;

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

			virtual ISASConnection ^ CreateConnection(System::String ^ module_name, System::String ^ invoker_name, ISASErrorCollector ^ ec);
		private:
			SASConnectorObj_priv * priv;
		};
	}
}

#pragma warning( pop ) 
