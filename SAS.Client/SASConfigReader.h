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

#include <sasCore/configreader.h>

namespace SAS {
	namespace Client {

		interface class ISASErrorCollector;

		public interface class ISASConfigReader
		{
		public:
			virtual bool GetEntryAsString(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] System::String ^% ret, ISASErrorCollector ^ ec) abstract;
			virtual bool GetEntryAsString(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] System::String ^% ret, System::String ^ defaultValue, ISASErrorCollector ^ ec) abstract;

			virtual bool GetEntryAsStringList(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] array<System::String^> ^% ret, ISASErrorCollector ^ ec) abstract;
			virtual bool GetEntryAsStringList(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] array<System::String^> ^% ret, array<System::String^> ^ defaultValue, ISASErrorCollector ^ ec) abstract;

			virtual bool GetStringEntry(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] System::String ^% ret, ISASErrorCollector ^ ec) abstract;
			virtual bool GetStringEntry(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] System::String ^% ret, System::String ^ defaultValue, ISASErrorCollector ^ ec) abstract;

			virtual bool GetStringListEntry(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] array<System::String^> ^% ret, ISASErrorCollector ^ ec) abstract;
			virtual bool GetStringListEntry(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] array<System::String^> ^% ret, array<System::String^> ^ defaultValue, ISASErrorCollector ^ ec) abstract;

			virtual bool GetNumberEntry(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] long long % ret, ISASErrorCollector ^ ec) abstract;
			virtual bool GetNumberEntry(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] long long % ret, long long defaultvalue, ISASErrorCollector ^ ec) abstract;

			virtual bool GetBoolEntry(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] bool % ret, ISASErrorCollector ^ ec) abstract;
			virtual bool GetBoolEntry(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] bool % ret, bool defaultvalue, ISASErrorCollector ^ ec) abstract;
		};


		public ref class SASConfigReader abstract : public ISASConfigReader
		{
		public:
			virtual bool GetEntryAsString(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] System::String ^% ret, ISASErrorCollector ^ ec);
			virtual bool GetEntryAsString(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] System::String ^% ret, System::String ^ defaultValue, ISASErrorCollector ^ ec);

			virtual bool GetEntryAsStringList(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] array<System::String^> ^% ret, ISASErrorCollector ^ ec) abstract;
			virtual bool GetEntryAsStringList(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] array<System::String^> ^% ret, array<System::String^> ^ defaultValue, ISASErrorCollector ^ ec);

			virtual bool GetStringEntry(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] System::String ^% ret, ISASErrorCollector ^ ec);
			virtual bool GetStringEntry(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] System::String ^% ret, System::String ^ defaultValue, ISASErrorCollector ^ ec);

			virtual bool GetStringListEntry(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] array<System::String^> ^% ret, ISASErrorCollector ^ ec);
			virtual bool GetStringListEntry(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] array<System::String^> ^% ret, array<System::String^> ^ defaultValue, ISASErrorCollector ^ ec);

			virtual bool GetNumberEntry(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] long long % ret, ISASErrorCollector ^ ec);
			virtual bool GetNumberEntry(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] long long % ret, long long defaultvalue, ISASErrorCollector ^ ec);

			virtual bool GetBoolEntry(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] bool % ret, ISASErrorCollector ^ ec);
			virtual bool GetBoolEntry(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] bool % ret, bool defaultvalue, ISASErrorCollector ^ ec);
		};

	}

	struct WConfigReader_priv;
	class WConfigReader : public ConfigReader
	{
	public:
		WConfigReader(Client::ISASConfigReader ^ mobj);
		virtual ~WConfigReader();

		virtual bool getEntryAsString(const std::string & path, std::string & ret, ErrorCollector & ec) final;
		virtual bool getEntryAsString(const std::string & path, std::string & ret, const std::string & defaultValue, ErrorCollector & ec) final;

		virtual bool getEntryAsStringList(const std::string & path, std::vector<std::string> & ret, ErrorCollector & ec) final;
		virtual bool getEntryAsStringList(const std::string & path, std::vector<std::string> & ret, const std::vector<std::string> & defaultValue, ErrorCollector & ec) final;

		virtual bool getStringEntry(const std::string & path, std::string & ret, ErrorCollector & ec) final;
		virtual bool getStringEntry(const std::string & path, std::string & ret, const std::string & defaultValue, ErrorCollector & ec) final;

		virtual bool getStringListEntry(const std::string & path, std::vector<std::string> & ret, ErrorCollector & ec) final;
		virtual bool getStringListEntry(const std::string & path, std::vector<std::string> & ret, const std::vector<std::string> & defaultValue, ErrorCollector & ec) final;

		virtual bool getNumberEntry(const std::string & path, long long & ret, ErrorCollector & ec) final;
		virtual bool getNumberEntry(const std::string & path, long long & ret, long long defaultvalue, ErrorCollector & ec) final;

		virtual bool getBoolEntry(const std::string & path, bool & ret, ErrorCollector & ec) final;
		virtual bool getBoolEntry(const std::string & path, bool & ret, bool defaultvalue, ErrorCollector & ec) final;

	private:
		WConfigReader_priv * priv;
	};
}

#pragma warning( pop ) 
