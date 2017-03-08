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

#include "SASConfigReader.h"
#include "SASErrorCollector.h"

#include "macros.h"

#include <msclr/gcroot.h>

using namespace msclr;

namespace SAS {
	namespace Client {

		//virtual 
		bool SASConfigReader::GetEntryAsString(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] System::String ^% ret, ISASErrorCollector ^ ec)
		{
			array<System::String^> ^ tmp;
			if (!GetStringListEntry(path, tmp, ec) || !tmp->Length)
				return false;
			ret = tmp[0];
			return true;
		}

		//virtual 
		bool SASConfigReader::GetEntryAsString(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] System::String ^% ret, System::String ^ defaultValue, ISASErrorCollector ^ ec)
		{
			if (!GetEntryAsString(path, ret, ec))
				ret = defaultValue;
			return true;
		}

		//virtual 
		bool SASConfigReader::GetEntryAsStringList(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] array<System::String^> ^% ret, array<System::String^> ^ defaultValue, ISASErrorCollector ^ ec)
		{
			if (!GetEntryAsStringList(path, ret, ec))
				ret = defaultValue;
			return true;
		}

		//virtual 
		bool SASConfigReader::GetStringEntry(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] System::String ^% ret, ISASErrorCollector ^ ec)
		{
			return GetEntryAsString(path, ret, ec);
		}
		
		//virtual 
		bool SASConfigReader::GetStringEntry(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] System::String ^% ret, System::String ^ defaultValue, ISASErrorCollector ^ ec)
		{
			return GetEntryAsString(path, ret, defaultValue, ec);
		}

		//virtual 
		bool SASConfigReader::GetStringListEntry(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] array<System::String^> ^% ret, ISASErrorCollector ^ ec)
		{
			return GetEntryAsStringList(path, ret, ec);
		}
		
		//virtual 
		bool SASConfigReader::GetStringListEntry(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] array<System::String^> ^% ret, array<System::String^> ^ defaultValue, ISASErrorCollector ^ ec)
		{
			return GetEntryAsStringList(path, ret, defaultValue, ec);
		}

		//virtual 
		bool SASConfigReader::GetNumberEntry(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] long long % ret, ISASErrorCollector ^ ec)
		{
			System::String ^ tmp;
			if (!GetEntryAsString(path, tmp, ec))
				return false;
			ret = System::Convert::ToInt64(tmp);
			return true;
		}
		
		//virtual 
		bool SASConfigReader::GetNumberEntry(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] long long % ret, long long defaultvalue, ISASErrorCollector ^ ec)
		{
			System::String ^ tmp;
			if (!GetEntryAsString(path, tmp, ec))
				ret = defaultvalue;
			else
				ret = System::Convert::ToInt64(tmp);
			return true;
		}

		//virtual 
		bool SASConfigReader::GetBoolEntry(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] bool % ret, ISASErrorCollector ^ ec)
		{
			System::String ^ tmp;
			if (!GetEntryAsString(path, tmp, ec))
				return false;
			ret = tmp->ToLower() == "true" || tmp != "0" || System::Convert::ToInt64(tmp) != 0;
			return true;
		}
		
		//virtual 
		bool SASConfigReader::GetBoolEntry(System::String ^ path, [System::Runtime::InteropServices::OutAttribute] bool % ret, bool defaultvalue, ISASErrorCollector ^ ec)
		{
			System::String ^ tmp;
			if (!GetEntryAsString(path, tmp, ec))
				ret = defaultvalue;
			else
				ret = tmp->ToLower() == "true" || tmp != "0" || System::Convert::ToInt64(tmp) != 0;
			return true;
		}
	}

	struct WConfigReader_priv
	{
		WConfigReader_priv(Client::ISASConfigReader ^ mobj_) : mobj(mobj_)
		{ }

		gcroot<Client::ISASConfigReader ^> mobj;
	};

	WConfigReader::WConfigReader(Client::ISASConfigReader ^ mobj) : priv(new WConfigReader_priv(mobj))
	{ }

	WConfigReader::~WConfigReader()
	{
		delete priv;
	}

	bool WConfigReader::getEntryAsString(const std::string & path, std::string & ret, ErrorCollector & ec)
	{
		System::String ^ tmp;
		if (!priv->mobj->GetEntryAsString(TO_MSTR(path), tmp, gcnew Client::SASErrorCollectorObj(ec)))
			return false;
		ret = TO_STR(tmp);
		return true;
	}

	bool WConfigReader::getEntryAsString(const std::string & path, std::string & ret, const std::string & defaultValue, ErrorCollector & ec)
	{
		System::String ^ tmp;
		if (!priv->mobj->GetEntryAsString(TO_MSTR(path), tmp, TO_MSTR(defaultValue), gcnew Client::SASErrorCollectorObj(ec)))
			return false;
		ret = TO_STR(tmp);
		return true;
	}

	bool WConfigReader::getEntryAsStringList(const std::string & path, std::vector<std::string> & ret, ErrorCollector & ec)
	{
		array<System::String ^> ^ tmp;
		if (!priv->mobj->GetEntryAsStringList(TO_MSTR(path), tmp, gcnew Client::SASErrorCollectorObj(ec)))
			return false;
		ret.resize(tmp->Length);
		for (int i(0), l(ret.size()); i < l; ++i)
		{
			auto t = tmp[i];
			ret[i] = TO_STR(t);
		}
		return true;
	}

	bool WConfigReader::getEntryAsStringList(const std::string & path, std::vector<std::string> & ret, const std::vector<std::string> & defaultValue, ErrorCollector & ec)
	{
		auto dv_tmp = gcnew array<System::String ^>(defaultValue.size());
		for (int i(0), l(ret.size()); i < l; ++i)
			dv_tmp[i] = TO_MSTR(defaultValue[i]);

		array<System::String ^> ^ tmp;

		if (!priv->mobj->GetEntryAsStringList(TO_MSTR(path), tmp, dv_tmp, gcnew Client::SASErrorCollectorObj(ec)))
			return false;
		ret.resize(tmp->Length);
		for (int i(0), l(ret.size()); i < l; ++i)
		{ 
			auto t = tmp[i];
			ret[i] = TO_STR(t);
		}
		return true;
	}

	bool WConfigReader::getStringEntry(const std::string & path, std::string & ret, ErrorCollector & ec)
	{
		System::String ^ tmp;
		if (!priv->mobj->GetStringEntry(TO_MSTR(path), tmp, gcnew Client::SASErrorCollectorObj(ec)))
			return false;
		ret = TO_STR(tmp);
		return true;
	}

	bool WConfigReader::getStringEntry(const std::string & path, std::string & ret, const std::string & defaultValue, ErrorCollector & ec)
	{
		System::String ^ tmp;
		if (!priv->mobj->GetStringEntry(TO_MSTR(path), tmp, TO_MSTR(defaultValue), gcnew Client::SASErrorCollectorObj(ec)))
			return false;
		ret = TO_STR(tmp);
		return true;
	}

	bool WConfigReader::getStringListEntry(const std::string & path, std::vector<std::string> & ret, ErrorCollector & ec)
	{
		array<System::String ^> ^ tmp;
		if (!priv->mobj->GetStringListEntry(TO_MSTR(path), tmp, gcnew Client::SASErrorCollectorObj(ec)))
			return false;
		ret.resize(tmp->Length);
		for (int i(0), l(ret.size()); i < l; ++i)
		{
			auto t = tmp[i];
			ret[i] = TO_STR(t);
		}
		return true;
	}

	bool WConfigReader::getStringListEntry(const std::string & path, std::vector<std::string> & ret, const std::vector<std::string> & defaultValue, ErrorCollector & ec)
	{
		auto dv_tmp = gcnew array<System::String ^>(defaultValue.size());
		for (int i(0), l(ret.size()); i < l; ++i)
			dv_tmp[i] = TO_MSTR(defaultValue[i]);

		array<System::String ^> ^ tmp;

		if (!priv->mobj->GetStringListEntry(TO_MSTR(path), tmp, dv_tmp, gcnew Client::SASErrorCollectorObj(ec)))
			return false;
		ret.resize(tmp->Length);
		for (int i(0), l(ret.size()); i < l; ++i)
		{
			auto t = tmp[i];
			ret[i] = TO_STR(t);
		}
		return true;
	}

	bool WConfigReader::getNumberEntry(const std::string & path, long long & ret, ErrorCollector & ec)
	{
		long long tmp;
		if (!priv->mobj->GetNumberEntry(TO_MSTR(path), tmp, gcnew Client::SASErrorCollectorObj(ec)))
			return false;
		ret = tmp;
		return true;
	}

	bool WConfigReader::getNumberEntry(const std::string & path, long long & ret, long long defaultvalue, ErrorCollector & ec)
	{
		long long tmp;
		if (!priv->mobj->GetNumberEntry(TO_MSTR(path), tmp, defaultvalue, gcnew Client::SASErrorCollectorObj(ec)))
			return false;
		ret = tmp;
		return true;
	}

	bool WConfigReader::getBoolEntry(const std::string & path, bool & ret, ErrorCollector & ec)
	{
		bool tmp;
		if (!priv->mobj->GetBoolEntry(TO_MSTR(path), tmp, gcnew Client::SASErrorCollectorObj(ec)))
			return false;
		ret = tmp;
		return true;
	}

	bool WConfigReader::getBoolEntry(const std::string & path, bool & ret, bool defaultvalue, ErrorCollector & ec)
	{
		bool tmp;
		if (!priv->mobj->GetBoolEntry(TO_MSTR(path), tmp, defaultvalue, gcnew Client::SASErrorCollectorObj(ec)))
			return false;
		ret = tmp;
		return true;
	}

}
