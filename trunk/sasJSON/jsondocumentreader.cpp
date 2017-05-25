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

#include "include/sasJSON/jsondocumentreader.h"
#include "include/sasJSON/jsondocument.h"

namespace SAS {

	struct JSONDocumentReader_priv
	{
		struct Docuemnt : public JSONDocument
		{
			Docuemnt(const rapidjson::Document & doc_) : doc(doc_)
			{ }

			virtual rapidjson::Document * obj() const final
			{
				return (rapidjson::Document*)&doc;
			}

			const rapidjson::Document & doc;

		} doc;

		JSONDocumentReader_priv(const rapidjson::Document & doc_) : doc(doc_)
		{ }

	};


	JSONDocumentReader::JSONDocumentReader(const rapidjson::Document & doc) : JSONReader(), priv(new JSONDocumentReader_priv(doc))
	{ }

	JSONDocumentReader::~JSONDocumentReader()
	{
		delete priv;
	}

	const JSONDocument & JSONDocumentReader::document() const
	{
		return priv->doc;
	}

}
