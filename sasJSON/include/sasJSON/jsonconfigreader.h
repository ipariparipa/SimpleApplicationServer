/*
This file is part of sasJSON.

sasJSON is free software: you can redistribute it and/or modify
it under the terms of the Lesser GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

sasJSON is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with sasJSON.  If not, see <http://www.gnu.org/licenses/>
*/

#ifndef sasJSON__JSONCONFIGREADER_H_
#define sasJSON__JSONCONFIGREADER_H_

#include "config.h"
#include "jsonreader.h"
#include <sasCore/configreader.h>

namespace SAS {

	struct JSONConfigReader_priv;
	class SAS_JSON__CLASS JSONConfigReader : public ConfigReader
	{
	public:
		JSONConfigReader(const JSONReader & reader);
		virtual ~JSONConfigReader();

		virtual bool getEntryAsString(const std::string & path, std::string & ret, ErrorCollector & ec) override;
		virtual bool getEntryAsString(const std::string & path, std::string & ret, const std::string & defaultValue, ErrorCollector & ec) override;

		virtual bool getEntryAsStringList(const std::string & path, std::vector<std::string> & ret, ErrorCollector & ec) override;
		virtual bool getEntryAsStringList(const std::string & path, std::vector<std::string> & ret, const std::vector<std::string> & defaultValue, ErrorCollector & ec) override;

		virtual bool getNumberEntry(const std::string & path, long long & ret, ErrorCollector & ec) override;
		virtual bool getNumberEntry(const std::string & path, long long & ret, long long defaultvalue, ErrorCollector & ec) override;

		virtual bool getBoolEntry(const std::string & path, bool & ret, ErrorCollector & ec) override;
		virtual bool getBoolEntry(const std::string & path, bool & ret, bool defaultvalue, ErrorCollector & ec) override;

	private:
		JSONConfigReader_priv * priv;
	};

}

#endif // sasJSON__JSONCONFIGREADER_H_
