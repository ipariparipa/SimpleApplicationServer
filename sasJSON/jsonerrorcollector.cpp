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

#include "include/sasJSON/jsonerrorcollector.h"

namespace SAS {

	struct JSONErrorCollector_priv
	{
		JSONErrorCollector_priv(rapidjson::Document::AllocatorType & alloc_) : alloc(alloc_), errs(rapidjson::kArrayType)
		{ }

		rapidjson::Document::AllocatorType & alloc;
		rapidjson::Value errs;
	};

	JSONErrorCollector::JSONErrorCollector(rapidjson::Document::AllocatorType & alloc) : 
		SAS::ErrorCollector(), priv(new JSONErrorCollector_priv(alloc))
	{ }

	JSONErrorCollector::~JSONErrorCollector()
	{
		delete priv;
	}

	rapidjson::Value & JSONErrorCollector::errors() const
	{
		return priv->errs;
	}

	void JSONErrorCollector::append(long errorCode, const std::string & errorText)
	{
		rapidjson::Value _obj(rapidjson::kObjectType);
		rapidjson::Value _errorCode(rapidjson::kNumberType);
		_errorCode.SetInt64(errorCode);
		rapidjson::Value _errorText(rapidjson::kStringType);
		_errorText.SetString(errorText.length() ? errorText.c_str() : "(empty)", priv->alloc);
		_obj.AddMember("error_code", _errorCode, priv->alloc);
		_obj.AddMember("error_text", _errorText, priv->alloc);
		priv->errs.PushBack(_obj, priv->alloc);
	}

}
