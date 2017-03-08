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

#ifndef sasTCL__tcldatareader_h
#define sasTCL__tcldatareader_h

#include "config.h"
#include <sasCore/defines.h>

#include <vector>
#include <list>
#include <map>
#include <string>

namespace SAS {

	class ErrorCollector;

	struct TCLDataReader_priv;
	class SAS_TCL__CLASS TCLDataReader
	{
		SAS_COPY_PROTECTOR(TCLDataReader)
	public:
		TCLDataReader();
		~TCLDataReader();

		bool read(const std::vector<char> & data, ErrorCollector & ec);

		short version() const;
		const std::list<std::string> & tclResults() const;
		const std::map<std::string, std::vector<char>> & blobs() const;

	private:
		TCLDataReader_priv * priv;
	};

}

#endif // sasTCL__tcldatareader_h
