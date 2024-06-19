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

#ifndef sasJSON__JSONINVOKER_H_
#define sasJSON__JSONINVOKER_H_

#include "config.h"
#include <sasCore/invoker.h>
#include <rapidjson/document.h>

#include <string>

namespace SAS {

struct JSONFunction
{
	virtual inline ~JSONFunction() { }

	struct SAS_JSON__CLASS RetVal
	{
		inline RetVal(rapidjson::Document & doc) : alloc(doc.GetAllocator()), value(doc)
		{ }

		inline RetVal(rapidjson::Document::AllocatorType & alloc_) : alloc(alloc_), _value(rapidjson::kObjectType), value(_value)
		{ }

		rapidjson::Document::AllocatorType & alloc;

	private:
		rapidjson::Value _value;

	public:
		rapidjson::Value & value;

	};

	virtual bool operator() (const rapidjson::Value & arguments, RetVal & ret, SAS::ErrorCollector & ec) = 0;
};


struct JSONInvoker_priv;

class SAS_JSON__CLASS JSONInvoker : public SAS::Invoker
{
	SAS_COPY_PROTECTOR(JSONInvoker);

public:
	JSONInvoker(const std::string & name);
	virtual ~JSONInvoker();

	virtual Status invoke(const std::vector<char> & input, std::vector<char> & output, SAS::ErrorCollector & ec) override;

	void addFunction(const std::string & name, JSONFunction * function);

private:
	JSONInvoker_priv * priv;
};

}

#endif /* sasJSON__JSONINVOKER_H_ */
