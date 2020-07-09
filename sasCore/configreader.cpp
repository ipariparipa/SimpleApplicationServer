/*
    This file is part of sasCore.

    sasCore is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasCore is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasCore.  If not, see <http://www.gnu.org/licenses/>
 */

#include "include/sasCore/configreader.h"
#include "include/sasCore/errorcollector.h"
#include "include/sasCore/logging.h"
#include "stdlib.h"

namespace SAS {

	ConfigReader::~ConfigReader()
	{ }

	bool ConfigReader::getEntryAsString(const std::string & path, std::string & ret, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		std::vector<std::string> tmp;
		if(!getEntryAsStringList(path, tmp, ec))
			return false;
		if(!tmp.size())
		{
			ret.clear();
			SAS_LOG_DEBUG(Logging::getLogger("SAS.ConfigReader"), "configuration entry '" + path + "' is found but not set");
		}
		else
			ret = tmp.front();
		return true;
	}

	bool ConfigReader::getEntryAsString(const std::string & path, std::string & ret, const std::string & defaultValue, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		std::vector<std::string> tmp_dv;
		tmp_dv.push_back(defaultValue);
		std::vector<std::string> tmp;
		if(!getEntryAsStringList(path, tmp, tmp_dv, ec))
			return false;
		ret = tmp.size() ? tmp.front() : defaultValue;
		return true;
	}

	bool ConfigReader::getStringEntry(const std::string & path, std::string & ret, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		return getEntryAsString(path, ret, ec);
	}

	bool ConfigReader::getStringEntry(const std::string & path, std::string & ret, const std::string & defaultValue, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		return getEntryAsString(path, ret, defaultValue, ec);
	}

	bool ConfigReader::getStringListEntry(const std::string & path, std::vector<std::string> & ret, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		return getEntryAsStringList(path, ret, ec);
	}

	bool ConfigReader::getStringListEntry(const std::string & path, std::vector<std::string> & ret, const std::vector<std::string> & defaultValue, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		return getEntryAsStringList(path, ret, defaultValue, ec);
	}

	bool ConfigReader::getNumberEntry(const std::string & path, long long & ret, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		std::string tmp;
		if(!getEntryAsString(path, tmp, ec))
			return false;
		return toNumber(tmp, ret, ec);
	}

	bool ConfigReader::getNumberEntry(const std::string & path, long long & ret, long long defaultvalue, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		std::string tmp, tmp_dv;
		if (!toString(defaultvalue, tmp_dv, ec))
			return false;
		if(!getEntryAsString(path, tmp, tmp_dv, ec))
			return false;
		return toNumber(tmp, ret, ec);
	}

	bool ConfigReader::getBoolEntry(const std::string & path, bool & ret, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		std::string tmp;
		if(!getEntryAsString(path, tmp, ec))
			return false;
		return toBool(tmp, ret, ec);
	}

	bool ConfigReader::getBoolEntry(const std::string & path, bool & ret, bool defaultvalue, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		std::string tmp, tmp_dv;
		if (!toString(defaultvalue, tmp_dv, ec))
			return false;
		if(!getEntryAsString(path, tmp, tmp_dv, ec))
			return false;
		return toBool(tmp, ret, ec);
	}

	bool ConfigReader::toBool(const std::string & str, bool & ret, ErrorCollector & ec) const
	{
        (void)ec;
		ret = str == "true" || atol(str.c_str());
		return true;
	}

	bool ConfigReader::toNumber(const std::string & str, long long & ret, ErrorCollector & ec) const
	{
        (void)ec;
        ret = atoll(str.c_str());
		return true;
	}

	bool ConfigReader::toString(bool v, std::string & ret, ErrorCollector & ec) const
	{
        (void)ec;
        ret = v ? "true" : "false";
		return true;
	}

	bool ConfigReader::toString(long long v, std::string & ret, ErrorCollector & ec) const
	{
        (void)ec;
        ret = std::to_string(v);
		return true;
	}

}
