/*
    This file is part of sasSQLClient.

    sasSQLClient is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasSQLClient is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasSQLClient.  If not, see <http://www.gnu.org/licenses/>
 */

#include "sc_module.h"
#include <sasCore/logging.h>
#include <sasCore/application.h>
#include <sasCore/configreader.h>
#include <sasCore/objectregistry.h>
#include <sasCore/session.h>
#include <sasCore/invoker.h>
#include <sasSQL/sqlconnector.h>
#include <sasSQL/sqlstatement.h>
#include <sasSQL/sqlvariant.h>
#include <sasTCL/tclinvoker.h>
#include <sasTCL/tclerrorcollector.h>
#include <sasTCL/tcllisthandler.h>

#include <memory>
#include <sstream>
#include <cstring>
#include <mutex>

namespace SAS { namespace SQLClient {

	class SQLTCLInvoker : public TCLInvoker
	{
	public:
		inline SQLTCLInvoker(SQLConnector * conn_, const std::string & name) : TCLInvoker(name), conn(conn_)
		{ }
	protected:

		static int _sql_query(ClientData obj_, Tcl_Interp * interp,  int argc, const char *argv[])
		{
			auto obj = (SQLTCLInvoker*)obj_;

			TCLErrorCollector ec(interp);

			if (argc != 2)
			{
				ec.add(-1, "sql_run: invalid arguments");
				Tcl_SetObjResult(interp, ec.errors().obj());
				return TCL_ERROR;
			}

			SAS_LOG_NDC();
			std::unique_ptr<SAS::SQLStatement> stmt(obj->conn->createStatement(ec));
			if (!stmt)
			{
				Tcl_SetObjResult(interp, ec.errors().obj());
				return TCL_ERROR;
			}

			if (!stmt->prepare(argv[1], ec))
			{
				Tcl_SetObjResult(interp, ec.errors().obj());
				return TCL_ERROR;
			}

			if (!stmt->exec(ec))
			{
				Tcl_SetObjResult(interp, ec.errors().obj());
				return TCL_ERROR;
			}

			TCLListHandler ret(interp);
			
			std::unique_lock<TCLInvoker::BlobHandler> __blob_locker(*obj->blobHandler());
			int blob_idx(0);
			std::vector<SQLVariant> data;
			while (stmt->fetch(data, ec))
			{
				TCLListHandler row(interp);
				for (auto & d : data)
				{
					TCLListHandler field(interp);
					switch (d.type())
					{
					case SQLDataType::None:
						field.append("none");
						break;
					case SQLDataType::String:
						field.append("string");
						break;
					case SQLDataType::Number:
						field.append("number");
						break;
					case SQLDataType::Real:
						field.append("real");
						break;
					case SQLDataType::DateTime:
						field.append("datetime");
						break;
					case SQLDataType::Blob:
						field.append("blob");
						break;
					}
					if (d.isNull())
						field.append("(null)");
					else
					{
						switch (d.type())
						{
						case SQLDataType::None:
							field.append("(none)");
							break;
						case SQLDataType::String:
						case SQLDataType::Number:
						case SQLDataType::Real:
						case SQLDataType::DateTime:
							field.append(d.toString());
							break;
						case SQLDataType::Blob:
						{
							std::string blob_name = "blob#" + std::to_string(blob_idx++);
							field.append(blob_name);
							std::vector<char> * blob;
							obj->blobHandler()->setBlob(blob_name, blob);
							size_t blob_size;
							auto blob_data = d.asBlob(blob_size);
							blob->resize(blob_size);
							memcpy(blob->data(), blob_data, blob_size);
							break;
						}
						}
					}

					row.append(field);
				}
				ret.append(row);
			}

			Tcl_SetObjResult(interp, ret.obj());
			return TCL_OK;
		}

		virtual inline void init(Tcl_Interp *interp) override
		{
			Tcl_CreateCommand(interp, "sql_query", _sql_query, this, 0);
		}
	private:
		SQLConnector * conn;
	};

class SC_Session_Invoker : public Session, public Invoker
{
public:
	SC_Session_Invoker(SQLConnector * conn_, const std::string & mod_name_, SessionID id) : Session(id),
		mod_name(mod_name_), conn(conn_),
		logger(Logging::getLogger("SAS.SQLClient.SC_Session_Invoker." + mod_name_))
	{ }

	virtual ~SC_Session_Invoker()
	{ }

	virtual Status invoke(const std::vector<char> & input, std::vector<char> & output, ErrorCollector & ec) final
	{
		SAS_LOG_NDC();
		std::unique_ptr<SAS::SQLStatement> stmt(conn->createStatement(ec));
		if (!stmt)
			return Invoker::Status::Error;

		std::string in_str;
		in_str.append(input.data(), input.size());
		if (!stmt->prepare(in_str, ec))
			return Invoker::Status::Error;

		if(!stmt->exec(ec))
			return Invoker::Status::Error;

		std::stringstream ss;
		std::vector<SQLVariant> data;
		while(stmt->fetch(data, ec))
		{
			for(auto & d : data)
				ss << d.toString() << "\t";
			ss << std::endl;
		}
		std::string out_str(ss.str());
		output.resize(out_str.length());
		std::memcpy(output.data(), out_str.data(), output.size());
		return Invoker::Status::OK;
	}

protected:
	virtual Invoker * getInvoker(const std::string & name, ErrorCollector & ec) final
	{
		SAS_LOG_NDC();
		if (name == "plain_text")
			return this;
		else if (name == "tcl")
		{
			if (!tclinv.get())
			{
				tclinv.reset(new SQLTCLInvoker(conn, mod_name));
				if (!tclinv->init(ec))
				{
					tclinv.release();
					return nullptr;
				}
			}
			return tclinv.get();
		}

		auto err = ec.add(-1, "unsupported invoker type: '" + name + "'");
		SAS_LOG_ERROR(logger, err);
		return nullptr;
	}

private:
	std::string mod_name;
	SQLConnector * conn;
	Logging::LoggerPtr logger;
	std::unique_ptr<TCLInvoker> tclinv;
};

struct SC_Module_priv
{
	SC_Module_priv() : conn(nullptr)
	{ }

	std::string name;
	Logging::LoggerPtr logger;
	SQLConnector * conn;
};

SC_Module::SC_Module(const std::string & name) : Module(), priv(new SC_Module_priv)
{
	priv->logger = Logging::getLogger("SAS.SQLClient.SC_Module." + (priv->name = name));
}

SC_Module::~SC_Module()
{
	delete priv;
}

std::string SC_Module::description() const
{
	return "SQLClient Module";
}

std::string SC_Module::version() const
{
	return "1.0";
}

std::string SC_Module::name() const
{
	return priv->name;
}

bool SC_Module::init(Application * app, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	long long default_session_lifetime;
	std::string prefix = "SAS/SQL_CLIENT/" + priv->name;
	if(!app->configReader()->getNumberEntry(prefix + "/DEFAULT_SESSION_LIFETIME", default_session_lifetime, 10, ec))
		return false;

	if(!SessionManager::init((long)default_session_lifetime, ec))
		return false;

	std::string conn_name;
	if(!app->configReader()->getStringEntry(prefix + "/SQL_CONNECTOR", conn_name, ec))
	{
		auto err = ec.add(-1, "connector name is not configured");
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}

	if(!(priv->conn = app->objectRegistry()->getObject<SQLConnector>(SAS_OBJECT_TYPE_SQL__CONNECTOR, conn_name, ec)))
		return false;

	if(!priv->conn->connect(ec))
		return false;

	return true;
}

Session * SC_Module::createSession(SessionID id, ErrorCollector & ec)
{
	return new SC_Session_Invoker(priv->conn, priv->name, id);
}

}}
