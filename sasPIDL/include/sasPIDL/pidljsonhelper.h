/*
    This file is part of sasPIDL.

    sasPIDL is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasPIDL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasPIDL.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef INCLUDE_SASPIDL_PIDLJSONHELPER_H_
#define INCLUDE_SASPIDL_PIDLJSONHELPER_H_

#include "config.h"
#include <rapidjson/document.h>
#include <sasCore/defines.h>

#include <vector>
#include <string>

namespace SAS {

	class ErrorCollector;

	class SAS_PIDL__CLASS PIDLJSONHelper
	{
		SAS_COPY_PROTECTOR(PIDLJSONHelper)
		struct Priv;
		Priv * priv;
	public:
		PIDLJSONHelper(const std::string & name);
		~PIDLJSONHelper();

		bool parse(std::vector<char> & buffer, rapidjson::Document & doc, ErrorCollector & ec);
		bool parseInsitu(std::vector<char> & buffer, rapidjson::Document & doc, ErrorCollector & ec);
		bool accept(const rapidjson::Value & root, std::vector<char> & data, ErrorCollector & ec);
	};
}

#endif /* INCLUDE_SASPIDL_PIDLJSONHELPER_H_ */
