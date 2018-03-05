/*
This file is part of sasTCL.

sasTCL is free software: you can redistribute it and/or modify
it under the terms of the Lesser GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

sasTCL is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with sasTCL.  If not, see <http://www.gnu.org/licenses/>
*/

#ifndef sasTCL__tclconfigreader_h
#define sasTCL__tclconfigreader_h

#include "config.h"
#include <sasCore/configreader.h>

namespace SAS {

	class SAS_TCL__CLASS TCLConfigReader : public ConfigReader
	{
		struct Priv;
		Priv * priv;
	public:
		TCLConfigReader();
		virtual ~TCLConfigReader();

		bool init(const std::string & tclfile, ErrorCollector & ec);
		bool init(const std::string & tclfile, const std::string & getter_function, ErrorCollector & ec);

		virtual bool getEntryAsStringList(const std::string & path, std::vector<std::string> & ret, ErrorCollector & ec) override;
		virtual bool getEntryAsStringList(const std::string & path, std::vector<std::string> & ret, const std::vector<std::string> & defaultValue, ErrorCollector & ec) override;

		virtual bool getEntryAsString(const std::string & path, std::string & ret, ErrorCollector & ec) override;
		virtual bool getEntryAsString(const std::string & path, std::string & ret, const std::string & defaultValue, ErrorCollector & ec) override;

	protected:
		virtual bool toBool(const std::string & str, bool & ret, ErrorCollector & ec) const override;
		virtual bool toString(bool v, std::string & ret, ErrorCollector & ec) const override;
	};

}

#endif // sasTCL__tclconfigreader_h
