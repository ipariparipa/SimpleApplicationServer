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

#include "SASTCLDataWriter.h"
#include "SASErrorCollector.h"
#include "SASBinData.h"
#include "errorcodes.h"

#include "macros.h"
#include <vector>
#include <string>
#include <algorithm>

namespace SAS {
	namespace Client {

		ref struct SASTCLDataWriter_priv
		{
			SASTCLDataWriter_priv(short ver_) : data(gcnew SASBinData()), ver(ver_)
			{ }

			SASBinData ^ data;
			short ver;
		};

		SASTCLDataWriter::SASTCLDataWriter(short ver) : priv(gcnew SASTCLDataWriter_priv(ver))
		{
			auto & in = priv->data->data();
			in.resize(2);
			uint16_t tmp_16 = ver;
			memcpy(in.data(), &tmp_16, sizeof(uint16_t));
		}

		bool SASTCLDataWriter::AddScript(System::String ^ script_, ISASErrorCollector ^ ec)
		{
			auto script = TO_STR(script_);
			auto & in = priv->data->data();

			switch (priv->ver)
			{
			case 1:
			{
				std::vector<char> header(4 + sizeof(uint32_t));
				char * header_data = header.data();
				memcpy(header_data, "TCLS", 4);
				header_data += 4;
				uint32_t tmp_32 = script.size();
				memcpy(header_data, &tmp_32, sizeof(uint32_t));
				header_data += sizeof(uint32_t);

				in.insert(std::end(in), header.begin(), header.end());
				in.insert(std::end(in), script.begin(), script.end());
				break;
			}
			default:
				ec->Add(SAS_CLIENT__ERROR__TCL_DATA_WRITER__UNSUPPORTED_VERSION, System::String::Format("unsupported data version: {0}", priv->ver));
				return false;
			}
			return true;
		}

		bool SASTCLDataWriter::AddBlobSetter(System::String ^ blob_name_, array<System::Byte> ^ data, ISASErrorCollector ^ ec)
		{
			auto blob_name = TO_STR(blob_name_);
			auto & in = priv->data->data();
			std::vector<char> blob(data->Length);
			for (int i(0), l(data->Length); i < l; ++i)
				blob[i] = data[i];

			switch (priv->ver)
			{
			case 1:
			{
				std::vector<char> header(4 + sizeof(uint16_t) + sizeof(uint32_t));
				char * header_data = header.data();
				memcpy(header_data, "BADD", 4);
				header_data += 4;

				uint32_t tmp_32 = sizeof(uint16_t) + blob_name.size() + blob.size();
				memcpy(header_data, &tmp_32, sizeof(uint32_t));
				header_data += sizeof(uint32_t);

				uint16_t tmp_16 = blob_name.size();
				memcpy(header_data, &tmp_16, sizeof(uint16_t));
				header_data += sizeof(uint16_t);

				in.insert(std::end(in), header.begin(), header.end());
				in.insert(std::end(in), blob_name.begin(), blob_name.end());
				in.insert(std::end(in), blob.begin(), blob.end());
				break;
			}
			default:
				ec->Add(SAS_CLIENT__ERROR__TCL_DATA_WRITER__UNSUPPORTED_VERSION, System::String::Format("unsupported data version: {0}", priv->ver));
				return false;
			}
			return true;
		}

		bool SASTCLDataWriter::AddBlobGetter(ISASErrorCollector ^ ec)
		{
			return AddBlobGetter(nullptr, ec);
		}

		bool SASTCLDataWriter::AddBlobGetter(System::String ^ blob_name_, ISASErrorCollector ^ ec)
		{
			auto blob_name = TO_STR(blob_name_);
			auto & in = priv->data->data();

			switch (priv->ver)
			{
			case 1:
			{
				std::vector<char> header(4 + sizeof(uint16_t) + sizeof(uint32_t));
				char * header_data = header.data();
				memcpy(header_data, "BGET", 4);
				header_data += 4;

				uint32_t tmp_32 = sizeof(uint16_t) + blob_name.size();
				memcpy(header_data, &tmp_32, sizeof(uint32_t));
				header_data += sizeof(uint32_t);

				uint16_t tmp_16 = blob_name.size();
				memcpy(header_data, &tmp_16, sizeof(uint16_t));
				header_data += sizeof(uint16_t);

				in.insert(std::end(in), header.begin(), header.end());
				in.insert(std::end(in), blob_name.begin(), blob_name.end());
				break;
			}
			default:
				ec->Add(SAS_CLIENT__ERROR__TCL_DATA_WRITER__UNSUPPORTED_VERSION, System::String::Format("unsupported data version: {0}", priv->ver));
				return false;
			}
			return true;
		}

		bool SASTCLDataWriter::AddBlobRemover(ISASErrorCollector ^ ec)
		{
			return AddBlobRemover(nullptr, ec);
		}

		bool SASTCLDataWriter::AddBlobRemover(System::String ^ blob_name_, ISASErrorCollector ^ ec)
		{
			auto blob_name = TO_STR(blob_name_);
			auto & in = priv->data->data();

			switch (priv->ver)
			{
			case 1:
			{
				std::vector<char> header(4 + sizeof(uint16_t) + sizeof(uint32_t));
				char * header_data = header.data();
				memcpy(header_data, "BREM", 4);
				header_data += 4;

				uint32_t tmp_32 = sizeof(uint16_t) + blob_name.size();
				memcpy(header_data, &tmp_32, sizeof(uint32_t));
				header_data += sizeof(uint32_t);

				uint16_t tmp_16 = blob_name.size();
				memcpy(header_data, &tmp_16, sizeof(uint16_t));
				header_data += sizeof(uint16_t);

				in.insert(std::end(in), header.begin(), header.end());
				in.insert(std::end(in), blob_name.begin(), blob_name.end());
				break;
			}
			default:
				ec->Add(SAS_CLIENT__ERROR__TCL_DATA_WRITER__UNSUPPORTED_VERSION, System::String::Format("unsupported data version: {0}", priv->ver));
				return false;
			}
			return true;
		}

		//property 
		SASBinData ^ SASTCLDataWriter::BinData::get()
		{
			return priv->data;
		}
	}
}
