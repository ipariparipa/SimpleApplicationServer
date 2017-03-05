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
