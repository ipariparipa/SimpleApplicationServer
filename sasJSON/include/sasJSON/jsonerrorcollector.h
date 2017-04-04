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

#ifndef sasJSON__JSONERROCOLLECTOR_H_
#define sasJSON__JSONERROCOLLECTOR_H_

#include "config.h"
#include <sasCore/errorcollector.h>

#include <rapidjson/document.h>

namespace SAS {

	struct JSONErrorCollector_priv;
	class SAS_JSON__CLASS JSONErrorCollector : public SAS::ErrorCollector
	{
		JSONErrorCollector(rapidjson::Document::AllocatorType & alloc_);

		const rapidjson::Value & errors() const;

	protected:
		virtual void append(long errorCode, const std::string & errorText) override;

	private:
		JSONErrorCollector_priv * priv;
	};

}

#endif // sasJSON__JSONERROCOLLECTOR_H_
