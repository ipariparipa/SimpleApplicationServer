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

#ifndef sasJSON__JSONDOCUMENT_H_
#define sasJSON__JSONDOCUMENT_H_

#include "config.h"
#include <rapidjson/document.h>

namespace SAS {

	class JSONDocument
	{
	public:
		virtual ~JSONDocument() { }

		rapidjson::Document & operator * () { return *obj(); }
		const rapidjson::Document & operator * () const { return *obj(); }

		rapidjson::Document * operator -> () { return obj(); }
		const rapidjson::Document * operator -> () const { return obj(); }

	protected:
		virtual rapidjson::Document * obj() const = 0;
	};

}

#endif //sasJSON__JSONDOCUMENT_H_
