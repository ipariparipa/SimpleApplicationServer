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

#include <msclr/gcroot.h>

#include "SASAppSettingsConfigReader.h"
#include "SASErrorCollector.h"

using namespace msclr;

namespace SAS {
	namespace Client {

		struct SASAppSettingsReader_priv
		{
			gcroot<System::Configuration::AppSettingsSection^> config;
		};

		SASAppSettingsReader::SASAppSettingsReader() : priv(new SASAppSettingsReader_priv)
		{ }

		SASAppSettingsReader::!SASAppSettingsReader()
		{
			delete priv;
		}

		bool SASAppSettingsReader::Init(ISASErrorCollector ^ ec)
		{

			if (priv->config)
				return true;
			try
			{
				if (!(priv->config = System::Configuration::ConfigurationManager::OpenExeConfiguration(System::Configuration::ConfigurationUserLevel::None)->AppSettings))
				{
					ec->Add(-1, "'appSettings' section is not found.");
					return false;
				}
			}
			catch (System::Exception ^ ex)
			{
				ec->Add(-1, "'" + ex->GetType()->Name + "' exception is caught: '" + ex->Message + "'");
				return false;
			}
			return true;
		}

		bool SASAppSettingsReader::GetEntryAsStringList(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] array<System::String ^> ^% ret, ISASErrorCollector ^ ec)
		{
			try
			{
				if (!Init(ec))
				{
					ret = nullptr;
					return false;
				}
				auto key = priv->config->Settings[path];
				if (key == nullptr)
				{
					ec->Add(-1, "config entry '" + path + "' is not found");
					ret = nullptr;
					return false;
				}

				ret = key->Value->Split('|');
			}
			catch (System::Exception ^ ex)
			{
				ec->Add(-1, "'" + ex->GetType()->Name + "' exception is caught: '" + ex->Message + "'");
				ret = nullptr;
				return false;
			}

			return true;
		}

	}
}
