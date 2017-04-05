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

#include "SASBinData.h"

#include <vector>
#include <string>
#include <algorithm>

#include "macros.h"

namespace SAS 
{
	namespace Client {

		struct SASBinData_priv
		{
			std::vector<char> data;
		};

		SASBinData::SASBinData(const std::vector<char> & data) : priv(new SASBinData_priv)
		{
			priv->data = data;
		}

		SASBinData::SASBinData() : priv(new SASBinData_priv)
		{ }

		SASBinData::SASBinData(array<System::Byte> ^ data) : priv(new SASBinData_priv)
		{
			priv->data.resize(data->Length);
			for (int i(0), l(data->Length); i < l; ++i)
				priv->data[i] = data[i];
		}

		SASBinData::SASBinData(System::String ^ data) : priv(new SASBinData_priv)
		{
			auto str = TO_STR(data);
			priv->data.resize(str.size());
			std::copy(str.begin(), str.end(), priv->data.begin());
		}

		SASBinData::!SASBinData()
		{
			delete priv;
		}

		//property 
		array<System::Byte> ^ SASBinData::AsByteArray::get()
		{
			auto ret = gcnew array<System::Byte>(priv->data.size());
			for (int i(0), l(priv->data.size()); i < l; ++i)
				ret[i] = priv->data[i];
			return ret;
		}

		System::String ^ SASBinData::ToString()
		{
			std::string str;
			str.append(priv->data.data(), priv->data.size());
			return str.length() ? gcnew System::String(str.c_str()) : System::String::Empty;
		}

		std::vector<char> & SASBinData::data()
		{
			return priv->data;
		}

	}
}
