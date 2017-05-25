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

#ifndef sasJSON__JSONDOCUMENTREADER_H_
#define sasJSON__JSONDOCUMENTREADER_H_

#include "jsonreader.h"
#include <rapidjson/document.h>

namespace SAS {

	struct JSONDocumentReader_priv;
	class JSONDocumentReader : public JSONReader
	{
		JSONDocumentReader(const rapidjson::Document & doc);
		virtual ~JSONDocumentReader();

		virtual const JSONDocument & document() const final;

	private:
		JSONDocumentReader_priv * priv;

	};
}

#endif // sasJSON__JSONDOCUMENTREADER_H_
