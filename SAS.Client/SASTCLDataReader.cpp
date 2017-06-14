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

#include "SASTCLDataReader.h"
#include "SASErrorCollector.h"
#include "SASBinData.h"
#include "errorcodes.h"

#include <msclr/gcroot.h>

#include "macros.h"
#include <vector>
#include <string>
#include <algorithm>

using namespace msclr;

namespace SAS {
	namespace Client {

		struct SASTCLDataReader_priv
		{
			gcroot<SASBinData^> data;
			short version;
			gcroot<System::Collections::Generic::List<System::String^>^> tclResults;
			gcroot<System::Collections::Generic::Dictionary<System::String^, array<System::Byte>^>^> blobs;
		};

		SASTCLDataReader::SASTCLDataReader() : priv(new SASTCLDataReader_priv)
		{
			priv->version = 0;
			priv->tclResults = gcnew System::Collections::Generic::List<System::String^>();
			priv->blobs = gcnew System::Collections::Generic::Dictionary<System::String^, array<System::Byte>^>();
		}

		SASTCLDataReader::!SASTCLDataReader()
		{
			delete priv;
		}

		bool SASTCLDataReader::Read(SASBinData ^ data, ISASErrorCollector ^ec)
		{
			System::GC::KeepAlive(data);
			auto & out = data->data();
			priv->tclResults = gcnew System::Collections::Generic::List<System::String^>();
			priv->blobs = gcnew System::Collections::Generic::Dictionary<System::String^, array<System::Byte>^>();
			size_t out_size = out.size();
			if (out_size < sizeof(uint16_t))
			{
				ec->Add(SAS_CLIENT__ERROR__TCL_DATA_READER__INVALID_DATA, "missing data version");
				return false;
			}
			const char * out_data = out.data();
			uint16_t version;
			memcpy(&version, out_data, sizeof(uint16_t));
			out_data += sizeof(uint16_t); out_size -= sizeof(uint16_t);

			switch (version)
			{
			case 1:
			{
				while (out_size)
				{
					if (out_size < 4 + sizeof(uint32_t))
					{
						ec->Add(SAS_CLIENT__ERROR__TCL_DATA_READER__INVALID_DATA, "missing or invalid data header");
						return false;
					}
					std::string format;
					format.append(out_data, 4);
					out_data += 4; out_size -= 4;

					uint32_t data_size;
					memcpy(&data_size, out_data, sizeof(uint32_t));
					out_data += sizeof(uint32_t); out_size -= sizeof(uint32_t);
					if (out_size < data_size)
					{
						ec->Add(SAS_CLIENT__ERROR__TCL_DATA_READER__INVALID_DATA, System::String::Format("invalid size information in data header: '{0}'", data_size));
						return false;
					}

					if (format == "TCLR")
					{
						std::string result;
						result.append(out_data, data_size);
						out_data += data_size; out_size -= data_size;
						priv->tclResults->Add(TO_MSTR(result));
					}
					else if (format == "BLOB")
					{
						if (data_size < sizeof(uint32_t))
						{
							ec->Add(SAS_CLIENT__ERROR__TCL_DATA_READER__INVALID_DATA, "BLOB: missing size information for blob name");
							return false;
						}

						uint16_t name_size;
						memcpy(&name_size, out_data, sizeof(uint16_t));
						out_data += sizeof(uint16_t); out_size -= sizeof(uint16_t);  data_size -= sizeof(uint16_t);

						if (data_size < name_size)
						{
							ec->Add(SAS_CLIENT__ERROR__TCL_DATA_READER__INVALID_DATA, "BLOB: missing blob name");
							return false;
						}

						std::string name;
						name.append(out_data, name_size);
						out_data += name_size; out_size -= name_size; data_size -= name_size;
						if (!name.length())
						{
							ec->Add(SAS_CLIENT__ERROR__TCL_DATA_READER__INVALID_DATA, "BLOB: blob name is not specified");
							return false;
						}
						std::vector<char> blob(data_size);
						memcpy(blob.data(), out_data, data_size);
						out_data += data_size; out_size -= data_size;

						auto ba = gcnew array<System::Byte>(blob.size());
						for (int i(0), l(blob.size()); i < l; ++i)
							ba[i] = blob[i];
						priv->blobs->Add(TO_MSTR(name), ba);
					}
				}

				break;
			}
			default:
				ec->Add(SAS_CLIENT__ERROR__TCL_DATA_READER__UNSUPPORTED_VERSION, System::String::Format("unsupported data version: {0}", version));
				return false;
			}
			return true;
		}

		//property 
		short SASTCLDataReader::Version::get()
		{
			return priv->version;
		}

		//property 
		System::Collections::Generic::List<System::String^> ^ SASTCLDataReader::TclResults::get()
		{
			return priv->tclResults;
		}

		//property 
		System::Collections::Generic::Dictionary<System::String^, array<System::Byte>^> ^ SASTCLDataReader::Blobs::get()
		{
			return priv->blobs;
		}

	}
}
