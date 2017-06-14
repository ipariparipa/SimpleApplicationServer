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

#include "include/sasJSON/jsonreader.h"
#include "include/sasJSON/jsondocument.h"
#include "include/sasJSON/errorcodes.h"

#include <sasCore/errorcollector.h>
#include <sasCore/logging.h>

#include <sstream>
#include <fstream>
#include <memory>
#include <vector>

namespace SAS {


	JSONReader::JSONReader() : priv(nullptr)
	{ }

	JSONReader::~JSONReader()
	{ }


	struct JSONStreamReader_priv
	{
		struct Document : public JSONDocument
		{
			Document() : _doc(new rapidjson::Document())
			{ }

			virtual rapidjson::Document * obj() const override
			{
				return _doc.get();
			}
		private:
			std::unique_ptr<rapidjson::Document> _doc;
		} doc;

		JSONStreamReader_priv() : logger(Logging::getLogger("SAS.JSONStreamReader"))
		{ }

		std::vector<char> buffer;

		Logging::LoggerPtr logger;
	};

	JSONStreamReader::JSONStreamReader() : JSONReader(), priv(new JSONStreamReader_priv)
	{ }

	JSONStreamReader::~JSONStreamReader()
	{
		delete priv;
	}

	bool JSONStreamReader::load(const std::string & filename, ErrorCollector & ec)
	{
		SAS_LOG_NDC();

		SAS_LOG_VAR(priv->logger, filename);

		std::ifstream file(filename);
		std::stringstream ss;
		if (file) {
			ss << file.rdbuf();
			file.close();
		}
		else
		{
			auto err = ec.add(SAS_JSON__ERROR__STREAM_READER__CANNOT_OPEN_FILE, "unable to open file");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}

		return set(ss.str(), ec);
	}

	bool JSONStreamReader::set(const std::string & json_stream, ErrorCollector & ec)
	{
		SAS_LOG_NDC();

		priv->buffer.resize(json_stream.length() + 1);
		memcpy(priv->buffer.data(), json_stream.c_str(), json_stream.length());

		if (priv->doc->ParseInsitu(priv->buffer.data()).HasParseError())
		{
			auto err = ec.add(SAS_JSON__ERROR__STREAM_READER__PARSE_ERROR, "JSON parse error (" + std::to_string(priv->doc->GetParseError()) + ")");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}
		return true;
	}

	const JSONDocument & JSONStreamReader::document() const
	{
		return priv->doc;
	}

}
