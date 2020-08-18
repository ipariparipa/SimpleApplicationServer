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

#ifndef INCLUDE_SASPIDL_PIDLJSONCLIENT_H_
#define INCLUDE_SASPIDL_PIDLJSONCLIENT_H_

#include "pidljsonhelper.h"
#include "errorcollector.h"

#include <sasCore/connector.h>
#include <pidlCore/jsontools.h>
#include <pidlCore/basictypes.h>

namespace SAS {

	template<class PIDL_JSON_Client_T>
	class PIDLJSONClient : public PIDL_JSON_Client_T
	{
		PIDLJSONHelper _helper;
		SAS::Connection * _conn;
	public:
		PIDLJSONClient(const std::string & name, SAS::Connection * conn) : PIDL_JSON_Client_T(), _helper(name), _conn(conn)
		{ }

		virtual ~PIDLJSONClient()
		{ }

        virtual PIDL::InvokeStatus _invoke(const rapidjson::Value & root, rapidjson::Document & retval, PIDL::ErrorCollector & ec) override
		{
			PIDL_SASErrorCollector sas_ec(ec);
			std::vector<char> in;
			if (!_helper.accept(root, in, sas_ec))
                return PIDL::InvokeStatus::MarshallingError;

			std::vector<char> out;
			auto stat = _conn->invoke(in, out, sas_ec);

			if(out.size())
			{
				if(!out.size() || out.back() != '\0')
					out.push_back('\0');
				if(!_helper.parse(out, retval, sas_ec))
                    return PIDL::InvokeStatus::MarshallingError;
			}

			switch(stat)
			{
			case Invoker::Status::OK:
                return PIDL::InvokeStatus::Ok;
			case Invoker::Status::Error:
                return PIDL::InvokeStatus::Error;
			case Invoker::Status::NotImplemented:
                return PIDL::InvokeStatus::NotImplemented;
			case Invoker::Status::FatalError:
				break;
			}
            return PIDL::InvokeStatus::FatalError;
		}

	};


}

#endif /* INCLUDE_SASPIDL_PIDLJSONCLIENT_H_ */
