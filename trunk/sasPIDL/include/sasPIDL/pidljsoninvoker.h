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
#include <pidlCore/jsontools.h>

#include <vector>
#include <string>
#include <memory>
#include "pidljsonhelper.h"

namespace SAS {

	template<class PIDL_JSON_Server_T>
	class PIDLJSONInvoker : public Invoker
	{
		std::shared_ptr<PIDL_JSON_Server_T> srv;
		PIDLJSONHelper helper;
	public:
		PIDLJSONInvoker(const std::string & name, const std::shared_ptr<PIDL_JSON_Server_T> & srv_) : 
			Invoker(), 
			srv(srv_),
			helper(name)
		{ }

		virtual ~PIDLJSONInvoker() = default;

		virtual Status invoke(const std::vector<char> & input, std::vector<char> & output, ErrorCollector & ec)
		{
			auto buffer = input;
			rapidjson::Document indoc;
			if(!helper.parseInsitu(buffer, indoc, ec))
				return Status::Error;

			SAS_PIDLErrorCollector _ec(ec);

			rapidjson::Document outdoc;
			outdoc.SetObject();
			switch(srv->_invoke(indoc, outdoc, _ec))
			{
			case PIDL::JSONTools::InvokeStatus::Ok:
				break;
			case PIDL::JSONTools::InvokeStatus::Error:
			case PIDL::JSONTools::InvokeStatus::MarshallingError:
			case PIDL::JSONTools::InvokeStatus::NotSupportedMarshaklingVersion:
				return Status::Error;
			case PIDL::JSONTools::InvokeStatus::FatalError:
				return Status::FatalError;
			case PIDL::JSONTools::InvokeStatus::NotImplemented:
				return Status::NotImplemented;
			}

			if(!helper.accept(outdoc, output, ec))
				return Status::Error;

			return Status::OK;
		}

	};

}

#endif /* INCLUDE_SASPIDL_PIDLJSONINVOKER_H_ */
