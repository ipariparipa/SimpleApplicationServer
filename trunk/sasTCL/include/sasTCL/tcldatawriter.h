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

#ifndef sasTCL__tcldatawriter_h
#define sasTCL__tcldatawriter_h

#include "config.h"
#include <sasCore/defines.h>

#include<vector>
#include <string>

namespace SAS {

	class ErrorCollector;

	struct TCLDataWriter_priv;
	class SAS_TCL__CLASS TCLDataWriter
	{
		SAS_COPY_PROTECTOR(TCLDataWriter)
	public:
		TCLDataWriter(short version);
		~TCLDataWriter();

		bool addScript(const std::string & script, ErrorCollector & ec);
		bool addBlobSetter(const std::string & blob_name, std::vector<char> & data, ErrorCollector & ec);
		bool addBlobGetter(const std::string & blob_name, ErrorCollector & ec);
		bool addBlobGetter(ErrorCollector & ec);
		bool addBlobRemover(const std::string & blob_name, ErrorCollector & ec);
		bool addBlobRemover(ErrorCollector & ec);

		const std::vector<char> & data() const;

	private:
		TCLDataWriter_priv * priv;
	};

}

#endif // sasTCL__tcldatawriter_h
