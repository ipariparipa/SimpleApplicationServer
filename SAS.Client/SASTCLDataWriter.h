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

namespace SAS {
	namespace Client {

		interface class ISASErrorCollector;
		ref class SASBinData;

		struct SASTCLDataWriter_priv;
		public ref class SASTCLDataWriter
		{
		public:
			SASTCLDataWriter(short ver);
			!SASTCLDataWriter();

			bool AddScript(System::String ^ script, ISASErrorCollector ^ ec);
			bool AddBlobSetter(System::String ^ blob_name, array<System::Byte> ^ data, ISASErrorCollector ^ ec);
			bool AddBlobGetter(System::String ^ blob_name, ISASErrorCollector ^ ec);
			bool AddBlobGetter(ISASErrorCollector ^ ec);
			bool AddBlobRemover(System::String ^ blob_name, ISASErrorCollector ^ ec);
			bool AddBlobRemover(ISASErrorCollector ^ ec);

			property SASBinData ^ BinData { SASBinData ^  get(); }

		private:
			SASTCLDataWriter_priv * priv;
		};

	}
}

#pragma warning( pop ) 
