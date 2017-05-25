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

#ifndef sasJSON__JSONREADER_H_
#define sasJSON__JSONREADER_H_

#include "config.h"

#include <string>

namespace SAS {

	class JSONDocument;
	class ErrorCollector;

	class JSONReader_Internal;
	struct JSONReader_priv;
	class SAS_JSON__CLASS JSONReader
	{
	public:
		JSONReader();
		virtual ~JSONReader();

		virtual const JSONDocument & document() const = 0;
	private:
		JSONReader_priv * priv;
	};

	struct JSONStreamReader_priv;
	class SAS_JSON__CLASS JSONStreamReader : public JSONReader
	{
	public:
		JSONStreamReader();
		virtual ~JSONStreamReader();

		bool load(const std::string & filename, ErrorCollector & ec);
		bool set(const std::string & json_stream, ErrorCollector & ec);

		virtual const JSONDocument & document() const final;

	private:
		JSONStreamReader_priv * priv;
	};

}

#endif sasJSON__JSONREADER_H_
