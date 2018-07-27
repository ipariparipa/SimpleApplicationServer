/*
    This file is part of sasPIDL.

    sasPIDL is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasPIDL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasPIDL.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef INCLUDE_SASPIDL_PIDLJSONINVOKER_H_
#define INCLUDE_SASPIDL_PIDLJSONINVOKER_H_

#include "config.h"
#include "errorcollector.h"

#include <sasCore/invoker.h>

#include <rapidjson/document.h>
#include <vector>
#include <string>
#include <memory>

namespace SAS {

	class SAS_PIDL__CLASS PIDLJSONInvoker_helper
	{
		SAS_COPY_PROTECTOR(PIDLJSONInvoker_helper)
		struct Priv;
		Priv * priv;
	public:
		PIDLJSONInvoker_helper(const std::string & name);
		~PIDLJSONInvoker_helper();

		bool parse(std::vector<char> & buffer, rapidjson::Document & doc, ErrorCollector & ec);
		bool accept(const rapidjson::Value & root, std::vector<char> & data, ErrorCollector & ec);
	};

	template<class PIDL_JSON_Server_T>
	class PIDLJSONInvoker : public Invoker
	{
		std::shared_ptr<PIDL_JSON_Server_T> srv;
		PIDLJSONInvoker_helper helper;
	public:
		PIDLJSONInvoker(const std::string & name, const std::shared_ptr<PIDL_JSON_Server_T> & srv_) : 
			Invoker(), 
			helper(name), 
			srv(srv_)
		{ }

		virtual ~PIDLJSONInvoker() = default;

		virtual Status invoke(const std::vector<char> & input, std::vector<char> & output, ErrorCollector & ec)
		{
			auto buffer = input;
			rapidjson::Document indoc;
			if(!helper.parse(buffer, indoc, ec))
				return Status::Error;

			SAS_PIDLErrorCollector _ec(ec);

			rapidjson::Document outdoc;
			outdoc.SetObject();
			switch(srv->_invoke(indoc, outdoc, _ec))
			{
			case PIDL_JSON_Server_T::_InvokeStatus::Ok:
				break;
			case PIDL_JSON_Server_T::_InvokeStatus::Error:
			case PIDL_JSON_Server_T::_InvokeStatus::MarshallingError:
				return Status::Error;
			case PIDL_JSON_Server_T::_InvokeStatus::FatalError:
				return Status::FatalError;
			case PIDL_JSON_Server_T::_InvokeStatus::NotImplemented:
				return Status::NotImplemented;
			}

			if(!helper.accept(outdoc, output, ec))
				return Status::Error;

			return Status::OK;
		}

	};

}

#endif /* INCLUDE_SASPIDL_PIDLJSONINVOKER_H_ */
