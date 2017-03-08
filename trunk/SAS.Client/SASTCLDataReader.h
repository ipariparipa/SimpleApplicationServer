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

namespace SAS{
	namespace Client{

		interface class ISASErrorCollector;
		ref class SASBinData;

		struct SASTCLDataReader_priv;
		public ref class SASTCLDataReader
		{
		public:
			SASTCLDataReader();
			!SASTCLDataReader();

			bool Read(SASBinData ^ data, ISASErrorCollector ^ec);

			property short Version { short get(); }
			property System::Collections::Generic::List<System::String^> ^ TclResults { System::Collections::Generic::List<System::String^> ^ get(); }
			property System::Collections::Generic::Dictionary<System::String^, array<System::Byte>^> ^ Blobs{ System::Collections::Generic::Dictionary<System::String^, array<System::Byte>^> ^ get(); }

		private:
			SASTCLDataReader_priv * priv;
		};

	}
}

#pragma warning( pop ) 
