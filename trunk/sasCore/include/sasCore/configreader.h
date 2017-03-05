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

#ifndef INCLUDE_SASCORE_CONFIGREADER_H_
#define INCLUDE_SASCORE_CONFIGREADER_H_

#include "defines.h"
#include <string>
#include <vector>

namespace SAS
{

	class ErrorCollector;

	class SAS_CORE__CLASS ConfigReader
	{
		SAS_COPY_PROTECTOR(ConfigReader)
	public:
		inline ConfigReader() { }
		virtual ~ConfigReader();
		virtual bool getEntryAsString(const std::string & path, std::string & ret, ErrorCollector & ec);
		virtual bool getEntryAsString(const std::string & path, std::string & ret, const std::string & defaultValue, ErrorCollector & ec);

		virtual bool getEntryAsStringList(const std::string & path, std::vector<std::string> & ret, ErrorCollector & ec) = 0;
		virtual bool getEntryAsStringList(const std::string & path, std::vector<std::string> & ret, const std::vector<std::string> & defaultValue, ErrorCollector & ec) = 0;

		virtual bool getStringEntry(const std::string & path, std::string & ret, ErrorCollector & ec);
		virtual bool getStringEntry(const std::string & path, std::string & ret, const std::string & defaultValue, ErrorCollector & ec);

		virtual bool getStringListEntry(const std::string & path, std::vector<std::string> & ret, ErrorCollector & ec);
		virtual bool getStringListEntry(const std::string & path, std::vector<std::string> & ret, const std::vector<std::string> & defaultValue, ErrorCollector & ec);

		virtual bool getNumberEntry(const std::string & path, long long & ret, ErrorCollector & ec);
		virtual bool getNumberEntry(const std::string & path, long long & ret, long long defaultvalue, ErrorCollector & ec);

		virtual bool getBoolEntry(const std::string & path, bool & ret, ErrorCollector & ec);
		virtual bool getBoolEntry(const std::string & path, bool & ret, bool defaultvalue, ErrorCollector & ec);
};

}

#endif /* INCLUDE_SASCORE_CONFIGREADER_H_ */
