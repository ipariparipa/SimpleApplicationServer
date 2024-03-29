/*
    This file is part of pidlsas.

    pidlsas is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    pidlsas is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with pidlsas.  If not, see <http://www.gnu.org/licenses/>
 */

#include <pidlBackend/job_json.h>

#include <string>
#include <iostream>
#include <fstream>
#include <memory>
#include <sstream>
#include <map>

#include <pidlCore/errorcollector.h>
#include <pidlCore/jsontools.h>
#include <pidlBackend/configreader.h>
#include <pidlBackend/readerfactory_json.h>
#include <pidlBackend/jsonreader.h>
#include <pidlBackend/cppcodegen.h>
#include <pidlBackend/cppcodegenfactory_json.h>

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

    virtual ~SASJSONReader() override = default;

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

class SAS_PIDL_JSONReaderFactory : public PIDL::ReaderFactory_JSON
{
	SAS::Connector * _conn;
public:
    SAS_PIDL_JSONReaderFactory(SAS::Connector * conn) :
		ReaderFactory_JSON(),
		_conn(conn)
	{ }

    virtual ~SAS_PIDL_JSONReaderFactory() = default;

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
        catch (PIDL::Exception & ex)
		{
            ex.get(ec);
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

class SAS_CPP_Logging : public PIDL::CPPCodeGenLogging
{
    PIDL::IncludeType _sasIncludeType;
    std::string _sasCoreIncludeDir;
public:
    SAS_CPP_Logging(PIDL::IncludeType sasIncludeType, const std::string & sasCoreIncludeDir) :
        _sasIncludeType(sasIncludeType),
        _sasCoreIncludeDir(sasCoreIncludeDir)
    { }

    std::vector<PIDL::Include> includes() const final override
    {
        std::vector<PIDL::Include> ret(1);
        ret[0] = std::make_pair(_sasIncludeType, _sasCoreIncludeDir.length() ? _sasCoreIncludeDir + "/logging.h" : "sasCore/logging.h");

        return ret;
    }

    std::string initLogger(const std::string & scope) const final override
    {
        return "SAS::Logging::getLogger(" + scope + ")";
    }

    std::string loggerType() const final override
    {
        return "SAS::Logging::LoggerPtr";
    }

    std::string loggingStart(const std::string & logger) const final override
    {
        (void)logger;
        return "SAS_LOG_NDC()";
    }

    std::string loggingAssert(const std::string & logger, const std::string & expression, const std::string & message) const final override
    {
        return "SAS_LOG_ASSERT(" + logger + ", " + expression + ", " + message + ")";
    }

    std::string loggingTrace(const std::string & logger, const std::string & message) const final override
    {
        return "SAS_LOG_TRACE(" + logger + ", " + message + ")";
    }

    std::string loggingDebug(const std::string & logger, const std::string & message) const final override
    {
        return "SAS_LOG_DEBUG(" + logger + ", " + message + ")";
    }

    std::string loggingInfo(const std::string & logger, const std::string & message) const final override
    {
        return "SAS_LOG_INFO(" + logger + ", " + message + ")";
    }

    std::string loggingWarning(const std::string & logger, const std::string & message) const final override
    {
        return "SAS_LOG_WARN(" + logger + ", " + message + ")";
    }

    std::string loggingError(const std::string & logger, const std::string & message) const final override
    {
        return "SAS_LOG_ERROR(" + logger + ", " + message + ")";
    }

    std::string loggingFatal(const std::string & logger, const std::string & message) const final override
    {
        return "SAS_LOG_FATAL(" + logger + ", " + message + ")";
    }

};

class SAS_CPP_Logging_Factory : public PIDL::CPPCodeGenLoggingFactory_JSON
{
    bool build(const rapidjson::Value & value, std::shared_ptr<PIDL::CPPCodeGenLogging> & ret, PIDL::ErrorCollector & ec)
    {
        if(!isValid(value))
        {
            ec.add(-1, "unexpected: invalid json object");
            return false;
        }

        std::string includeType_str, sasCoreIncludeDir;

        if(value.IsObject())
        {
            PIDL::JSONTools::getValue(value, "include_type", includeType_str);
            PIDL::JSONTools::getValue(value, "sas_core_include_dir", sasCoreIncludeDir);
        }

        PIDL::IncludeType includeType = PIDL::IncludeType::Local;
        if(!includeType_str.length() || includeType_str == "global")
            includeType = PIDL::IncludeType::GLobal;
        else if(includeType_str == "local")
            includeType = PIDL::IncludeType::Local;
        else
        {
            ec.add(-1, "unsupported include type: '"+includeType_str+"'");
            return false;
        }

        ret = std::make_shared<SAS_CPP_Logging>(includeType, sasCoreIncludeDir);

        return true;
    }

    bool isValid(const rapidjson::Value & value) const
    {
        std::string type;
        return ((value.IsString() && PIDL::JSONTools::getValue(value, type)) || (value.IsObject() && PIDL::JSONTools::getValue(value, "type", type))) && type == "sas_cpp";
    }

};

int main(int argc, char **argv)
{
	SAS::StreamErrorCollector<std::ostream> ec(std::cerr);
	SAS::SAS_PIDLErrorCollector pidl_ec(ec);

	SAS::Logging::init(argc, argv, ec);

	std::shared_ptr<std::istream> in;

    struct CLAConfigReader : public PIDL::ConfigReader
    {
        std::map<std::string, std::string> data;

        Status getAsString(const std::string & name, std::string & ret, PIDL::ErrorCollector & ec) final override
        {
            (void)ec;
            if(!data.count(name))
                return Status::NotFound;
            ret = data[name];
            return Status::OK;
        }

    };

    auto cr = std::make_shared<CLAConfigReader>();

	enum class Stat
	{
        None, File, Config
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
                std::cout << "-cfg <varname>=<value>" << std::endl;
				return 0;
			}
			if (a == "-stdin")
				in = std::shared_ptr<std::istream>(&std::cin, [](void*){});
			else if (a == "-file")
				stat = Stat::File;
            else if (a == "-cfg")
                stat = Stat::Config;
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
        case Stat::Config:
        {
            auto f = a.find('=');
            if(f == std::string::npos)
            {
                ec.add(-1, "invalid config format");
                return 1;
            }

            cr->data[a.substr(0, f)] = a.substr(f+1);
            stat = Stat::None;
        }
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

    std::string connector;
    if(!app.configReader()->getStringEntry("SAS/ADM_CONNECTOR", connector, "default", ec))
        return 1;

    auto job = std::make_shared<PIDL::Job_JSON>(cr);

	SAS::Connector * conn;
    if ((conn = app.objectRegistry()->getObject<SAS::Connector>(SAS_OBJECT_TYPE__CONNECTOR, connector, ec)))
    {
        job->factoryRegistry()->add(std::make_shared<SAS_PIDL_JSONReaderFactory>(conn));
    }

    job->factoryRegistry()->add(std::make_shared<SAS_CPP_Logging_Factory>());

	if (!job->build(*jsonreader.document(), pidl_ec))
		return 1;

	if (!job->run(pidl_ec))
		return 1;

	return 0;
}
