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

#pragma warning( push )
#pragma warning( disable: 4461 )

#include "SASBinData.h"

#include <vector>
#include <string>
#include <algorithm>

#include "macros.h"

#include <iostream>

namespace SAS 
{
	namespace Client {

		struct SASBinData_um_priv
		{
			std::vector<char> data;
		};

		ref struct SASBinData_priv
		{
			SASBinData_priv() : um(new SASBinData_um_priv)
			{ }

			!SASBinData_priv()
			{ delete um; }

			SASBinData_um_priv * um;
		};

		SASBinData::SASBinData(const std::vector<char> & data) : priv(gcnew SASBinData_priv)
		{
			priv->um->data = data;
		}

		SASBinData::SASBinData() : priv(gcnew SASBinData_priv)
		{ }

		SASBinData::SASBinData(array<System::Byte> ^ data) : priv(gcnew SASBinData_priv)
		{
			priv->um->data.resize(data->Length);
			for (int i(0), l(data->Length); i < l; ++i)
				priv->um->data[i] = data[i];
		}

		SASBinData::SASBinData(System::String ^ data) : priv(gcnew SASBinData_priv)
		{
			auto str = TO_STR(data);
			priv->um->data.resize(str.size());
			std::copy(str.begin(), str.end(), priv->um->data.begin());
		}

		//property 
		array<System::Byte> ^ SASBinData::AsByteArray::get()
		{
			auto ret = gcnew array<System::Byte>(priv->um->data.size());
			for (int i(0), l(priv->um->data.size()); i < l; ++i)
				ret[i] = priv->um->data[i];
			return ret;
		}

		System::String ^ SASBinData::ToString()
		{
			std::string str;
			str.append(priv->um->data.data(), priv->um->data.size());
			return str.length() ? gcnew System::String(str.c_str()) : System::String::Empty;
		}

		std::vector<char> & SASBinData::data()
		{
			return priv->um->data;
		}

	}
}

#pragma warning( pop )
