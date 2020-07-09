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

#include "include/sasPIDL/pidljsonhelper.h"

#include <sasCore/logging.h>

#include <sasCore/errorcollector.h>
#include <pidlCore/jsontools.h>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <algorithm>

namespace SAS {

	struct PIDLJSONHelper::Priv
	{
		Priv(const std::string & name) : logger(Logging::getLogger("PIDLJSONInvoker." + name))
		{ }

		Logging::LoggerPtr logger;
	};

	PIDLJSONHelper::PIDLJSONHelper(const std::string & name) : priv(new Priv(name))
	{ }

	PIDLJSONHelper::~PIDLJSONHelper()
	{
		delete priv;
	}

	bool PIDLJSONHelper::parseInsitu(std::vector<char> & buffer, rapidjson::Document & doc, ErrorCollector & ec)
	{
		if (!buffer.size())
		{
			doc.SetObject();
			return true;
		}
		if (buffer.back() != '\0')
			buffer.push_back('\0');
		if (doc.ParseInsitu(buffer.data()).HasParseError())
		{
			auto err = ec.add(-1, "JSON parse error (" + PIDL::JSONTools::getErrorText(doc.GetParseError()) + ")");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}
		return true;
	}

	bool PIDLJSONHelper::parse(std::vector<char> & buffer, rapidjson::Document & doc, ErrorCollector & ec)
	{
		if (doc.Parse(buffer.data()).HasParseError())
		{
			auto err = ec.add(-1, "JSON parse error (" + PIDL::JSONTools::getErrorText(doc.GetParseError()) + ")");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}
		return true;
	}

	bool PIDLJSONHelper::accept(const rapidjson::Value & root, std::vector<char> & data, ErrorCollector & ec)
	{
        (void)ec;
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		root.Accept(writer);

		data.resize(buffer.GetSize());
		memcpy(data.data(), buffer.GetString(), buffer.GetSize());

		return true;
	}

}
