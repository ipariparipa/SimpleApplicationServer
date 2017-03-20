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

#include "SASBasicApplication.h"
#include "SASAppSettingsConfigReader.h"

#include <msclr/gcroot.h>

using namespace msclr;

namespace SAS {
	namespace Client{

		struct SASBasicApplication_priv
		{
			gcroot<SASAppSettingsReader ^ > configReader;
		};

		SASBasicApplication::SASBasicApplication(array<System::String^> ^ args) : SASApplication(args), priv(new SASBasicApplication_priv)
		{
			priv->configReader = gcnew SASAppSettingsReader();
		}

		SASBasicApplication::SASBasicApplication() : SASApplication(), priv(new SASBasicApplication_priv)
		{
			priv->configReader = gcnew SASAppSettingsReader();
		}

		SASBasicApplication::!SASBasicApplication()
		{
			delete priv;
		}

		bool SASBasicApplication::Init(ISASErrorCollector ^ ec)
		{
			if (!priv->configReader->Init(ec))
				return false;
			return SASApplication::Init(ec);
		}

		//property 
		ISASConfigReader ^ SASBasicApplication::ConfigReader::get()
		{
			return priv->configReader;
		}

	}
}
