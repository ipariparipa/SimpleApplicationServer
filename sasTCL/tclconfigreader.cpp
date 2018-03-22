/*
This file is part of sasTCL.

sasTCL is free software: you can redistribute it and/or modify
it under the terms of the Lesser GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

sasTCL is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with sasTCL.  If not, see <http://www.gnu.org/licenses/>
*/

#include "include/sasTCL/tclconfigreader.h"
#include "include/sasTCL/tclerrorcollector.h"
#include "include/sasTCL/tclinterpinitilizer.h"
#include "include/sasTCL/tclexecutor.h"
#include "include/sasTCL/errorcodes.h"

#include <sasCore/errorcollector.h>
#include <sasCore/logging.h>
#include <sasTCLTools/tcllist.h>

#include <fstream>
#include <sstream>

namespace SAS {

	struct TCLConfigReader::Priv : public TCLInterpInitializer
	{
		Priv() :
			logger(Logging::getLogger("SAS.TCLConfigReader")),
			exec("TCLConfigReader")
		{ }

		virtual ~Priv() { }

		Logging::LoggerPtr logger;
		TCLExecutor exec;

		std::string getter_function;

	private:
		virtual void init(Tcl_Interp * interp) override
		{
			Tcl_CreateCommand(interp, "SAS::get_entry", _get_entry, this, 0);
		}

		static int _get_entry(ClientData obj_, Tcl_Interp * interp, int argc, const char *argv[])
		{
			SAS_LOG_NDC();

			auto obj = static_cast<Priv*>(obj_);
			SAS::TCLErrorCollector ec(interp);

			if (argc < 2)
			{
				auto err = ec.add(SAS_TCL__ERROR__CONFIG_READER__CANNOT_GET_ENTRY, "SAS::get_entry: invalid arguments");
				SAS_LOG_ERROR(obj->logger, err);
				Tcl_SetObjResult(interp, ec.errors().obj());
				return TCL_ERROR;
			}

			const char * val;;
			if (!(val = Tcl_GetVar(interp, argv[1], 0)))
			{
				if (argc < 3)
				{
					auto err = ec.add(SAS_TCL__ERROR__CONFIG_READER__ENTRY_NOT_FOUND, "SAS::get_entry: variable '" + std::string(argv[1]) + "' is not found");
					SAS_LOG_ERROR(obj->logger, err);
					Tcl_SetObjResult(interp, ec.errors().obj());
					return TCL_ERROR;
				}
				val = argv[2];
			}

			Tcl_SetObjResult(interp, SAS::TCLObjectRef(Tcl_NewStringObj(val, -1)));
			return TCL_OK;
		}
	};


	TCLConfigReader::TCLConfigReader() : priv(new Priv)
	{ }

	TCLConfigReader::~TCLConfigReader()
	{
		delete priv;
	}

	bool TCLConfigReader::init(const std::string & tclfile, ErrorCollector & ec)
	{
		return init(tclfile, "SAS::get_entry", ec);
	}

	bool TCLConfigReader::init(const std::string & tclfile, const std::string & getter_function, ErrorCollector & ec)
	{
		if (!getter_function.length())
		{
			auto err = ec.add(SAS_TCL__ERROR__CONFIG_READER__INVALID_ARGUMENT, "argument 'getter_function' cannot be empty");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}
		priv->getter_function = getter_function;
		if (!priv->exec.start(ec))
			return false;

		{
			TCLExecutor::Run run;
			run.operation = TCLExecutor::Run::Init;
			run.initer = priv;
			run.ec = &ec;
			if (!priv->exec.run(&run, ec))
				return false;
		}

		std::ifstream file;
		file.open(tclfile, std::ios_base::binary);
		if (file.fail())
		{
			auto err = ec.add(SAS_CORE__ERROR__CONFIG_READER__CANNOT_READ_CONFIG, "could not read TCL config file");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}

		TCLExecutor::Run run;
		run.script.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
		file.close();
		run.ec =  &ec;
		if (!priv->exec.run(&run, ec))
			return false;

		return run.isOK;
	}

	bool TCLConfigReader::getEntryAsStringList(const std::string & path, std::vector<std::string> & ret, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		TCLExecutor::Run run;
		run.ec = &ec;
		TCLList lst;
		lst << priv->getter_function << path;
		run.script = lst.toString();
		if (!priv->exec.run(&run, ec))
			return false;
		if (!run.isOK)
			return false;
		TCLList lst_res;
		if (!lst_res.fromString(run.result))
		{
			auto err = ec.add(SAS_TCL__ERROR__CONFIG_READER__INVALID_LIST_VALUE, "unexpected error: could not use result as valid TCL list");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}

		ret.resize(lst_res.length());
		for (size_t i(0), l(ret.size()); i < l; ++i)
			ret[i] = lst_res[i];

		return true;
	}

	bool TCLConfigReader::getEntryAsStringList(const std::string & path, std::vector<std::string> & ret, const std::vector<std::string> & defaultValue, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		TCLExecutor::Run run;
		run.ec = &ec;
		TCLList lst_dv;
		for (auto v : defaultValue)
			lst_dv << v;
		TCLList lst;
		lst << priv->getter_function << path << lst_dv;
		run.script = lst.toString();
		if (!priv->exec.run(&run, ec))
			return false;
		if (!run.isOK)
			return false;
		TCLList lst_res;
		if (!lst_res.fromString(run.result))
		{
			auto err = ec.add(SAS_TCL__ERROR__CONFIG_READER__INVALID_LIST_VALUE, "unexpected error: could not use result as valid TCL list");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}

		ret.resize(lst_res.length());
		for (size_t i(0), l(ret.size()); i < l; ++i)
			ret[i] = lst_res[i];

		return true;
	}

	bool TCLConfigReader::getEntryAsString(const std::string & path, std::string & ret, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		TCLExecutor::Run run;
		run.ec = &ec;
		TCLList lst;
		lst << priv->getter_function << path;
		run.script = lst.toString();
		if (!priv->exec.run(&run, ec))
			return false;
		if (!run.isOK)
			return false;

		ret = run.result;

		return true;
	}

	bool TCLConfigReader::getEntryAsString(const std::string & path, std::string & ret, const std::string & defaultValue, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		TCLExecutor::Run run;
		run.ec = &ec;
		TCLList lst;
		lst << priv->getter_function << path << defaultValue;
		run.script = lst.toString();
		if (!priv->exec.run(&run, ec))
			return false;
		if (!run.isOK)
			return false;

		ret = run.result;

		return true;
	}

	bool TCLConfigReader::toBool(const std::string & v, bool & ret, ErrorCollector & ec) const
	{
		ret = v.length() ? (std::stol(v) != 0) : false;
		return true;
	}

	bool TCLConfigReader::toString(bool v, std::string & ret, ErrorCollector & ec) const
	{
		ret = v ? "1" : "0";
		return true;
	}

}
