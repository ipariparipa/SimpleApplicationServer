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

#include "SASAppSettingsConfigReader.h"
#include "SASErrorCollector.h"
#include "errorcodes.h"
#include "sasCore/logging.h"
#include "macros.h"

namespace SAS {
	namespace Client {

		struct SASAppSettingsReader_upriv
		{
			SASAppSettingsReader_upriv() : logger(SAS::Logging::getLogger("SAS.SASAppSettingsReader"))
			{ }

			SAS::Logging::LoggerPtr logger;
		};

		ref struct SASAppSettingsReader_priv
		{
			SASAppSettingsReader_priv() : upriv(new SASAppSettingsReader_upriv)
			{ }

			!SASAppSettingsReader_priv()
			{
				delete upriv;
			}

			System::Configuration::AppSettingsSection ^  config;
			SASAppSettingsReader_upriv * upriv;
		};

		SASAppSettingsReader::SASAppSettingsReader() : priv(gcnew SASAppSettingsReader_priv)
		{ }

		bool SASAppSettingsReader::Init(ISASErrorCollector ^ ec)
		{

			if (priv->config)
				return true;
			try
			{
				if (!(priv->config = System::Configuration::ConfigurationManager::OpenExeConfiguration(System::Configuration::ConfigurationUserLevel::None)->AppSettings))
				{
					ec->Add(SAS_CLIENT__ERROR__CONFIG_READER__CANNOT_READ_CONFIG, "'appSettings' section is not found.");
					return false;
				}
			}
			catch (System::Exception ^ ex)
			{
				ec->Add(SAS_CLIENT__ERROR__CONFIG_READER__UNEXPECTED_ERROR, "'" + ex->GetType()->Name + "' exception is caught: '" + ex->Message + "'");
				return false;
			}
			return true;
		}

		bool SASAppSettingsReader::GetEntryAsStringList(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] array<System::String ^> ^% ret, ISASErrorCollector ^ ec)
		{
			SAS_LOG_NDC();
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
					auto err =  ec->Add(SAS_CLIENT__ERROR__CONFIG_READER__ENTRY_NOT_FOUND, "config entry '" + path + "' is not found");
					SAS_LOG_ERROR(priv->upriv->logger, TO_STR(err));
					ret = nullptr;
					return false;
				}

				ret = key->Value->Split('|');
			}
			catch (System::Exception ^ ex)
			{
				auto err = ec->Add(SAS_CLIENT__ERROR__CONFIG_READER__CANNOT_GET_ENTRY, "'" + ex->GetType()->Name + "' exception is caught: '" + ex->Message + "'");
				SAS_LOG_ERROR(priv->upriv->logger, TO_STR(err));
				ret = nullptr;
				return false;
			}

			return true;
		}

		bool SASAppSettingsReader::GetEntryAsStringList(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] array<System::String^> ^% ret, array<System::String^> ^ defaultValue, ISASErrorCollector ^ ec)
		{
			SAS_LOG_NDC();
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
					SAS_LOG_DEBUG(priv->upriv->logger, std::string() + "config entry '" + TO_STR(path) + "' is not found, use default value");
					ret = defaultValue;
					return true;
				}

				ret = key->Value->Split('|');
			}
			catch (System::Exception ^ ex)
			{
				auto err = ec->Add(SAS_CLIENT__ERROR__CONFIG_READER__CANNOT_GET_ENTRY, "'" + ex->GetType()->Name + "' exception is caught: '" + ex->Message + "'");
				SAS_LOG_ERROR(priv->upriv->logger, TO_STR(err));
				ret = nullptr;
				return false;
			}

			return true;
		}

	}
}
