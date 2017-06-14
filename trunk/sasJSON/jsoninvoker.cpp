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

#include "include/sasJSON/jsoninvoker.h"
#include "include/sasJSON/errorcodes.h"

#include <sasCore/logging.h>
#include <sasCore/errorcollector.h>

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace SAS {

	struct JSONInvoker_priv
	{
		JSONInvoker_priv(const std::string & name) : logger(SAS::Logging::getLogger("SAS.JSONInvoker."+name))
		{ }

		bool call(rapidjson::Value & function, JSONFunction::RetVal & ret, SAS::ErrorCollector & ec)
		{
			if(!function.IsObject())
			{
				auto err = ec.add(SAS_CORE__ERROR__INVOKER__TYPE_MISMATCH, "function call must be an object");
				SAS_LOG_ERROR(logger, err);
				return false;
			}

			if(!function.HasMember("function"))
			{
				auto err = ec.add(SAS_CORE__ERROR__INVOKER__INVALID_DATA, "'function' is not specified");
				SAS_LOG_ERROR(logger, err);
				return false;
			}

			auto & _name = function["function"];
			if(_name.IsNull() || !_name.IsString())
			{
				auto err = ec.add(SAS_CORE__ERROR__INVOKER__TYPE_MISMATCH, "'function' is not string");
				SAS_LOG_ERROR(logger, err);
				return false;
			}

			std::string name = _name.GetString();
			if(!functions.count(name))
			{
				auto err = ec.add(SAS_JSON__ERROR__INVOKER__UNSUPPORTED_FUNCTIONALITY, "unknown function: '" + name + "'");
				SAS_LOG_ERROR(logger, err);
				return false;
			}

			if(!function.HasMember("arguments"))
			{
				auto err = ec.add(SAS_CORE__ERROR__INVOKER__INVALID_DATA, "'arguments' for function '" + name + "' are not specified");
				SAS_LOG_ERROR(logger, err);
				return false;
			}
			auto & _args = function["arguments"];

			return (*functions[name])(_args, ret, ec);
		}


		SAS::Logging::LoggerPtr logger;
		std::map<std::string /*function name*/, JSONFunction*> functions;
	};

	JSONInvoker::JSONInvoker(const std::string & name) : SAS::Invoker(), priv(new JSONInvoker_priv(name))
	{
	}

	JSONInvoker::~JSONInvoker()
	{
		for(auto & f : priv->functions)
			delete f.second;
		delete priv;
	}

	JSONInvoker::Status JSONInvoker::invoke(const std::vector<char> & input, std::vector<char> & output, SAS::ErrorCollector & ec)
	{
		rapidjson::Document doc;
		std::vector<char> _input(input.size() + 1);
		memcpy(_input.data(), input.data(), input.size());
		if(doc.ParseInsitu(_input.data()).HasParseError())
		{
			auto err = ec.add(SAS_JSON__ERROR__INVOKER__INVALID_DATA, "JSON parse error (" + std::to_string(doc.GetParseError()) + ")");
			SAS_LOG_ERROR(priv->logger, err);
			return Status::Error;
		}
		rapidjson::Document ret_doc;
		ret_doc.Parse("{}");
		JSONFunction::RetVal rv(ret_doc);
		if(!priv->call(doc, rv, ec))
			return Status::Error;

		rapidjson::StringBuffer sb;
		rapidjson::Writer<rapidjson::StringBuffer> w(sb);
		ret_doc.Accept(w);

		output.resize(sb.GetSize());
		memcpy(output.data(), sb.GetString(), sb.GetSize());

		return Status::OK;
	}

	void JSONInvoker::addFunction(const std::string & name, JSONFunction * function)
	{
		priv->functions[name] = function;
	}

}
