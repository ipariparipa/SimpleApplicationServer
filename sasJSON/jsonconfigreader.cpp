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

#include "include/sasJSON/jsonconfigreader.h"
#include "include/sasJSON/jsondocument.h"
#include "include/sasJSON/jsonreader.h"

#include <sasCore/logging.h>
#include <sasCore/errorcollector.h>

#include <sasCore/tools.h>

#include <memory>
#include <list>
#include <fstream>
#include <sstream>

namespace SAS {

	struct JSONConfigReader_priv
	{
		JSONConfigReader_priv(const rapidjson::Document & doc_) : logger(Logging::getLogger("SAS.JSONConfigReader")), doc(doc_)
		{ }

		Logging::LoggerPtr logger;
		const rapidjson::Document & doc;

		const rapidjson::Value & _getValue(const rapidjson::Value & node, std::list<std::string> & path) const
		{
			static rapidjson::Value __null_value__;

			if (!node.IsObject())
				return __null_value__;

			auto & name = path.front();

			if (!node.HasMember(name.c_str()))
				return __null_value__;

			auto & ret = node[name.c_str()];
			path.pop_front();
			if (path.size())
				return _getValue(ret, path);
			return ret;
		}

		const rapidjson::Value & getValue(const std::string & path) const
		{
			SAS_LOG_NDC();
			auto tmp = str_split(path, '/');
			auto & ret = _getValue(doc, tmp);
			return ret;
		}
	};

	JSONConfigReader::JSONConfigReader(const JSONReader & reader) : ConfigReader(), priv(new JSONConfigReader_priv(*reader.document()))
	{ }

	JSONConfigReader::~JSONConfigReader()
	{
		delete priv;
	}

	bool JSONConfigReader::getEntryAsString(const std::string & path, std::string & ret, ErrorCollector & ec)
	{
		SAS_LOG_NDC();

		auto & v = priv->getValue(path);
		if (v.IsNull())
		{
			auto err = ec.add(-1, "unable to find config path: '" + path + "'");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}

		if (!v.IsString())
		{
			auto err = ec.add(-1, "config entry cannot be get as string");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}

		ret = v.GetString();

		return true;
	}

	bool JSONConfigReader::getEntryAsString(const std::string & path, std::string & ret, const std::string & defaultValue, ErrorCollector & ec)
	{
		SAS_LOG_NDC();

		auto & v = priv->getValue(path);
		if (v.IsNull())
		{
			ret = defaultValue;
			return true;
		}

		if (!v.IsString())
		{
			auto err = ec.add(-1, "config entry cannot be get as string");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}

		ret = v.GetString();

		return true;
	}


	bool JSONConfigReader::getEntryAsStringList(const std::string & path, std::vector<std::string> & ret, ErrorCollector & ec)
	{
		SAS_LOG_NDC();

		auto & v = priv->getValue(path);
		if (v.IsNull())
		{
			auto err = ec.add(-1, "unable to find config path: '" + path + "'");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}

		if (!v.IsArray())
		{
			auto err = ec.add(-1, "config entry cannot be get as array");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}

		ret.resize(v.Capacity());
		bool has_error(false);
		for (rapidjson::SizeType i = 0, l = v.Capacity(); i < l; ++i)
		{
			if(!v[i].IsString())
			{
				auto err = ec.add(-1, "value #"+std::to_string(i)+" cannot be get as string");
				SAS_LOG_ERROR(priv->logger, err);
				has_error = true;
			}
			else
				ret[i] = v[i].GetString();
		}

		return !has_error;
	}

	bool JSONConfigReader::getEntryAsStringList(const std::string & path, std::vector<std::string> & ret, const std::vector<std::string> & defaultValue, ErrorCollector & ec)
	{
		SAS_LOG_NDC();

		auto & v = priv->getValue(path);
		if (v.IsNull())
		{
			ret = defaultValue;
			return true;
		}

		if (!v.IsArray())
		{
			auto err = ec.add(-1, "config entry cannot be get as array");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}

		ret.resize(v.Capacity());
		bool has_error(false);
		for (rapidjson::SizeType i = 0, l = v.Capacity(); i < l; ++i)
		{
			if (!v[i].IsString())
			{
				auto err = ec.add(-1, "value #" + std::to_string(i) + " cannot be get as string");
				SAS_LOG_ERROR(priv->logger, err);
				has_error = true;
			}
			else
				ret[i] = v[i].GetString();
		}

		return !has_error;
	}


	bool JSONConfigReader::getNumberEntry(const std::string & path, long long & ret, ErrorCollector & ec)
	{
		SAS_LOG_NDC();

		auto & v = priv->getValue(path);
		if (v.IsNull())
		{
			auto err = ec.add(-1, "unable to find config path: '" + path + "'");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}

		if (!v.IsNumber())
		{
			auto err = ec.add(-1, "config entry cannot be get as number");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}

		ret = v.GetInt64();

		return true;
	}

	bool JSONConfigReader::getNumberEntry(const std::string & path, long long & ret, long long defaultvalue, ErrorCollector & ec)
	{
		SAS_LOG_NDC();

		auto & v = priv->getValue(path);
		if (v.IsNull())
		{
			ret = defaultvalue;
			return true;
		}

		if (!v.IsNumber())
		{
			auto err = ec.add(-1, "config entry cannot be get as number");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}

		ret = v.GetInt64();

		return true;
	}


	bool JSONConfigReader::getBoolEntry(const std::string & path, bool & ret, ErrorCollector & ec)
	{
		SAS_LOG_NDC();

		auto & v = priv->getValue(path);
		if (v.IsNull())
		{
			auto err = ec.add(-1, "unable to find config path: '" + path + "'");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}

		if (!v.IsBool())
		{
			auto err = ec.add(-1, "config entry cannot be get as boolean");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}

		ret = v.GetBool();

		return true;
	}

	bool JSONConfigReader::getBoolEntry(const std::string & path, bool & ret, bool defaultvalue, ErrorCollector & ec)
	{
		SAS_LOG_NDC();

		auto & v = priv->getValue(path);
		if (v.IsNull())
		{
			ret = defaultvalue;
			return true;
		}

		if (!v.IsBool())
		{
			auto err = ec.add(-1, "config entry cannot be get as boolean");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}

		ret = v.GetBool();

		return true;
	}

}
