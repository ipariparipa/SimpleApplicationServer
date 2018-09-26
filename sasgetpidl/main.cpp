/*
    This file is part of sasgetpidl.

    sasgetpidl is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasgetpidl is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasgetpidl.  If not, see <http://www.gnu.org/licenses/>
 */

#include <pidlBackend/job_json.h>

#include <string>
#include <iostream>
#include <fstream>
#include <memory>
#include <sstream>

#include <pidlCore/errorcollector.h>
#include <pidlCore/jsontools.h>
#include <pidlBackend/readerfactory_json.h>
#include <pidlBackend/jsonreader.h>

#include <sasBasics/logging.h>
#include <sasBasics/streamerrorcollector.h>
#include <sasCore/application.h>
#include <sasCore/objectregistry.h>
#include <sasCore/connector.h>
#include <sasJSON/jsonconfigreader.h>
#include <sasJSON/jsondocument.h>
#include <sasPIDL/errorcollector.h>
#include <sasPIDL/pidljsonclient.h>

#include "generated/pidladmin.h"

class SASApp : public SAS::Application
{
	SAS::JSONConfigReader cr;
	SAS::JSONConfigReader * cr_ptr;
public:
	SASApp(const SAS::JSONReader & reader, int argc, char **argv) : 
		SAS::Application(argc, argv), 
		cr(reader), cr_ptr(&cr)
	{ }

	virtual SAS::ConfigReader * configReader() final override
	{
		return cr_ptr;
	}
};

class SASJSONReader : public SAS::JSONReader, SAS::JSONDocument
{
	std::unique_ptr<rapidjson::Document> _doc;

public:

	SASJSONReader() : _doc(new rapidjson::Document)
	{ }

	virtual ~SASJSONReader() = default;

	virtual const SAS::JSONDocument & document() const final override
	{
		return *this;
	}

	bool parse(const std::string & json_data, SAS::ErrorCollector & ec)
	{
		if (_doc->Parse(json_data.c_str()).HasParseError())
		{
			ec.add(-1, std::string("JSON parse error (") + PIDL::JSONTools::getErrorText(_doc->GetParseError()) + ")");
			return false;
		}

		return true;
	}

protected:
	virtual rapidjson::Document * obj() const
	{
		return _doc.get();
	}
};

class PIDL_SAS_JSONReaderFactory : public PIDL::ReaderFactory_JSON
{
	SAS::Connector * _conn;
public:
	PIDL_SAS_JSONReaderFactory(SAS::Connector * conn) :
		ReaderFactory_JSON(),
		_conn(conn)
	{ }

	virtual ~PIDL_SAS_JSONReaderFactory() = default;

	virtual bool build(const rapidjson::Value & value, std::shared_ptr<PIDL::Reader> & ret, PIDL::ErrorCollector & ec) override
	{
		SAS::PIDL_SASErrorCollector sas_ec(ec);

		std::string module_name;
		if (!PIDL::JSONTools::getValue(value, "module", module_name))
		{
			ec << "module name is not specified";
			return false;
		}

		std::string invoker_name;
		if (!PIDL::JSONTools::getValue(value, "invoker", invoker_name))
		{
			ec << "invoker name is not specified";
			return false;
		}

		std::unique_ptr<SAS::Connection> conn(_conn->createConnection(module_name, invoker_name, sas_ec));
		if (!conn)
			return false;

		SAS::PIDLJSONClient<SAS::PIDLAdmin> adm("adm", conn.get());
		std::string str;

		try
		{
			str = adm.getPIDL();
		}
		catch (PIDL::Exception * ex)
		{
			ex->get(ec);
			return false;
		}

		ret = std::make_shared<PIDL::JSONReader>(str);
		return true;
	}

	virtual bool isValid(const rapidjson::Value & value) const override
	{
		std::string type_str;
		if (!value.IsObject() || !PIDL::JSONTools::getValue(value, "type", type_str))
			return false;

		return type_str == "sas_json";
	}
};

int main(int argc, char **argv)
{
	SAS::StreamErrorCollector<std::ostream> ec(std::cerr);
	SAS::SAS_PIDLErrorCollector pidl_ec(ec);

	SAS::Logging::init(argc, argv, ec);

	std::shared_ptr<std::istream> in;

	enum class Stat
	{
		None, File
	} stat = Stat::None;

	for (int i = 1; i < argc; ++i)
	{
		std::string a(argv[i]);
		switch (stat)
		{
		case Stat::None:
			if (a == "-help")
			{
				std::cout << "-help" << std::endl;
				std::cout << "-stdin" << std::endl;
				std::cout << "-file <filename>" << std::endl;
				return 0;
			}
			if (a == "-stdin")
				in = std::shared_ptr<std::istream>(&std::cin, [](void*){});
			else if (a == "-file")
				stat = Stat::File;
			else
			{
				//noop
			}
			break;
		case Stat::File:
			if (!a.length())
			{
				ec.add(-1, "filename is not specified");
				return 1;
			}
			in = std::make_shared<std::ifstream>(a);
			stat = Stat::None;
			break;
		}
	}

	if (stat != Stat::None)
	{
		ec.add(-1, "invalid parameters. use '-help'");
		return 1;
	}

	if (!in)
	{
		ec.add(-1, "input is not specified");
		return 1;
	}

	std::stringstream ss;
	ss << in->rdbuf();

	SASJSONReader jsonreader;
	jsonreader.parse(ss.str(), ec);

	SASApp app(jsonreader, argc, argv);

	if (!app.init(ec))
		return 1;

	SAS::Connector * conn;
	if (!(conn = app.objectRegistry()->getObject<SAS::Connector>(SAS_OBJECT_TYPE__CONNECTOR, "default", ec)))
		return 1;

	auto job = std::make_shared<PIDL::Job_JSON>();

	job->factoryRegistry()->add(std::make_shared<PIDL_SAS_JSONReaderFactory>(conn));

	if (!job->build(*jsonreader.document(), pidl_ec))
		return 1;

	if (!job->run(pidl_ec))
		return 1;

	return 0;
}
