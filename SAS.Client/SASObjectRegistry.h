
#pragma once
#pragma warning( push )
#pragma warning( disable: 4461 )

#include "SASObject.h"

namespace SAS {

	class ObjectRegistry;

	namespace Client {

		interface class ISASErrorCollector;
		interface class ISASObject;
		interface class ISASConnector;

		struct SASObjectRegistry_priv;
		public ref class SASObjectRegistry
		{
		internal:
			SASObjectRegistry(ObjectRegistry * obj);
		public:
			!SASObjectRegistry();

			ISASObject ^ getObject(System::String ^ type, System::String ^ name, ISASErrorCollector ^ ec);

			ISASConnector ^ GetConnector(System::String ^ name, ISASErrorCollector ^ ec);

			array<ISASObject^> ^ GetObjects(System::String ^ type, ISASErrorCollector ^ ec);

			array<ISASConnector^> ^ GetConnectors(ISASErrorCollector ^ ec);

		private:
			SASObjectRegistry_priv * priv;
		};

	}
}

#pragma warning( pop ) 
