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

#include "SASTextWriterErrorCollector.h"

#include <msclr/gcroot.h>

using namespace msclr;

namespace SAS {

	namespace Client {

		struct SASTextWriterErrorCollector_priv
		{
			gcroot<System::IO::TextWriter ^ > writer;
		};

		SASTextWriterErrorCollector::SASTextWriterErrorCollector(System::IO::TextWriter ^ writer) : priv(new SASTextWriterErrorCollector_priv)
		{
			priv->writer = writer;
		}

		SASTextWriterErrorCollector::!SASTextWriterErrorCollector()
		{
			delete priv;
		}

		void SASTextWriterErrorCollector::Add(long errorCode, System::String ^ errorText)
		{
			auto tmp = System::String::Format("[{0}] {1}", errorCode, errorText);
			priv->writer->WriteLine(tmp);
		}

	}
}
