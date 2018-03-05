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

#include "SASErrorCollector.h"

namespace SAS {

	namespace Client {

		ref struct SASTextWriterErrorCollector_priv;

		public ref class SASTextWriterErrorCollector : public ISASErrorCollector
		{
		public:
			SASTextWriterErrorCollector(System::IO::TextWriter ^ writer);

			virtual void Add(long errorCode, System::String ^ errorText);
		private:
			SASTextWriterErrorCollector_priv ^ priv;
		};
	}
}

#pragma warning( pop ) 
