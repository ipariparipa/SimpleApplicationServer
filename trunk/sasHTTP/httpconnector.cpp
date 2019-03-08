/*
This file is part of sasHTTP.

sasHTTP is free software: you can redistribute it and/or modify
it under the terms of the Lesser GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

sasHTTP is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with sasHTTP.  If not, see <http://www.gnu.org/licenses/>
*/

#include "httpconnector.h"
#include "httpcommon.h"

#include <sasCore/logging.h>
#include <sasCore/application.h>
#include <sasCore/thread.h>
#include <sasCore/tools.h>
#include <sasCore/configreader.h>
#include <sasCore/session.h>

#include <rapidjson/document.h>

#include <neon/ne_session.h>
#include <neon/ne_request.h>
#include <neon/ne_utils.h>
#include <neon/ne_uri.h>

#include <sstream>
#include <mutex>

#include "assert.h"

namespace SAS {

	struct HTTPConnectionOptions
	{
		std::string baseURL;
		std::string contentType;

		HTTPMethod method_invoke;
		HTTPMethod method_control;

		bool build(const std::string & path, ConfigReader * cr, ErrorCollector & ec)
		{
			if(!cr->getStringEntry(path + "/BASE_URL", baseURL, ec))
				return false;

			if(!cr->getStringEntry(path + "/CONTENT_TYPE", contentType, "application/octet-stream", ec))
				return false;

			auto str_to_method = [](const std::string & str) -> HTTPMethod
			{
				if (str == "PUT")
					return HTTPMethod::PUT;
				else if (str == "POST")
					return HTTPMethod::POST;
				else if (str == "GET")
					return HTTPMethod::GET;

				return HTTPMethod::None;
			};

			std::string tmp_str;

			if (!cr->getStringEntry(path + "/METHOD_INVOKE", tmp_str, "PUT", ec))
				return false;
			if ((method_invoke = str_to_method(tmp_str)) == HTTPMethod::None)
			{
				ec.add(-1, std::string() + "invalid value of 'METHOD_INVOKE': '" + tmp_str + "'");
				return false;
			}
			if (method_invoke == HTTPMethod::GET)
			{
				ec.add(-1, "'invoke' message cannot be handles with 'GET' method");
				return false;
			}

			if (!cr->getStringEntry(path + "/METHOD_CONTROL", tmp_str, "PUT", ec))
				return false;
			if ((method_control = str_to_method(tmp_str)) == HTTPMethod::None)
			{
				ec.add(-1, std::string() + "invalid value of 'METHOD_CONTROL': '" + tmp_str + "'");
				return false;
			}

			return true;
		}
	};

	class HTTPCaller
	{
		Logging::LoggerPtr _logger;
		std::string _module;
		std::mutex mut;
		HTTPConnectionOptions _options;

		ne_session *_sess = nullptr;
	public:
		HTTPCaller(const std::string & module, const std::string & name) :
			_logger(Logging::getLogger("SAS.HTTPCaller." + module + "." + name)),
			_module(module)
		{ }

		bool init(const HTTPConnectionOptions & options, ErrorCollector & ec)
		{
			SAS_LOG_NDC();
			std::unique_lock<std::mutex> __locker(mut);

			_options = options;

			ne_uri uri;
			ne_uri_parse(options.baseURL.c_str(), &uri);

			SAS_LOG_TRACE(_logger, "ne_session_create");
			_sess = ne_session_create(uri.scheme, uri.host, uri.port);
			SAS_LOG_TRACE(_logger, "ne_set_useragent");
			ne_set_useragent(_sess, "SAS/1.0");

			return true;
		}

		bool connect(ErrorCollector & ec)
		{
			// nothing to do
			return true;
		}

		bool disconnect(long timeout, ErrorCollector & ec)
		{
			// nothing to do
			return true;
		}

		void deinit()
		{
			SAS_LOG_NDC();
			std::unique_lock<std::mutex> __locker(mut);

			SAS_LOG_TRACE(_logger, "ne_close_connection");
			ne_close_connection(_sess);
			SAS_LOG_TRACE(_logger, "ne_session_destroy");
			ne_session_destroy(_sess);
		}

		bool msg_exchange(HTTPMethod method, /*in-out*/ SessionID & sid, const std::string & invoker, const std::string & mode, const std::vector<char> & input, std::vector<char> & output, Invoker::Status & status, ErrorCollector & ec)
		{
			SAS_LOG_NDC();
			std::unique_lock<std::mutex> __locker(mut);

			assert(_sess);

			ne_request * req = nullptr;
			switch (method)
			{
			case HTTPMethod::POST:
			case HTTPMethod::PUT:
				SAS_LOG_TRACE(_logger, "ne_request_create");
				req = ne_request_create(_sess, "PUT", ("/" + _module + "?mode=" + mode).c_str());
				SAS_LOG_TRACE(_logger, "ne_add_request_header");
				ne_add_request_header(req, "Content-type", _options.contentType.c_str());
				SAS_LOG_TRACE(_logger, "ne_add_request_header");
				ne_add_request_header(req, "Content-Length", std::to_string(input.size()).c_str());
				if (sid)
				{
					SAS_LOG_TRACE(_logger, "ne_add_request_header");
					ne_add_request_header(req, "SID", std::to_string((unsigned long long) sid).c_str());
				}

				if (invoker.length())
				{
					SAS_LOG_TRACE(_logger, "ne_add_request_header");
					ne_add_request_header(req, "Invoker", invoker.c_str());
				}

				SAS_LOG_TRACE(_logger, "ne_set_request_body_buffer");
				ne_set_request_body_buffer(req, input.data(), input.size());
				break;
			case HTTPMethod::GET:
				{
					std::string url = "/" + _module + "?mode=" + mode;
					if (sid)
						url += "&sid=" + std::to_string((unsigned long long) sid);
					if (invoker.length())
						url += "&invoker=" + invoker;

					SAS_LOG_TRACE(_logger, "ne_request_create");
					req = ne_request_create(_sess, "GET", url.c_str());
				}
				break;
			case HTTPMethod::None:
				{
					auto err = ec.add(-1, "unexpected: invaslid method specified");
					SAS_LOG_FATAL(_logger, err);
				}
				return false;
			}

			auto _accept = [](void *userdata, ne_request *req, const ne_status *st) -> int
			{
				return 1;
			};

			auto _reader = [](void *userdata, const char *buf, size_t len) -> int
			{
				auto output_buffer = (std::list<std::vector<char>>*) userdata;
				std::vector<char> _buff(len);
				memcpy(_buff.data(), buf, len);
				output_buffer->push_back(_buff);

				return 0;
			};

			std::list<std::vector<char>> _output_buffer;

			SAS_LOG_ASSERT(_logger, req, "HTTP request has not been created");

			SAS_LOG_TRACE(_logger, "ne_add_response_body_reader");
			ne_add_response_body_reader(req, _accept, _reader, &_output_buffer);

			if (ne_request_dispatch(req) != NE_OK)
			{
				auto err = ec.add(-1, std::string() + "error when dispatching HTTP request: '" + ne_get_error(_sess) + "'");
				SAS_LOG_ERROR(_logger, err);
				return false;
			}

			switch(ne_get_status(req)->code)
			{
			case 200: //OK
				{
					SAS_LOG_TRACE(_logger, "ne_get_response_header");
					const char * sid_str = ne_get_response_header(req, "SID");
					if(sid_str)
					{
						try
						{
							sid = std::stoull(sid_str);
						}
						catch(...)
						{
							auto err = ec.add(-1, std::string() + "unexpected error when converting session ID '" + sid_str + "'");
							SAS_LOG_ERROR(_logger, err);
							return false;
						}
					}
					status = Invoker::Status::OK;
					break;
				}
			case 500: //Internal Server Error
				status = Invoker::Status::Error;
				break;
			case 501: //Not Implemented
				status = Invoker::Status::NotImplemented;
				break;
			case 400: //Bad Request
				status = Invoker::Status::Error;
				break;
			}

			SAS_LOG_TRACE(_logger, "ne_end_request");
			ne_end_request(req);

			if (_output_buffer.size())
			{
				size_t size = 0;
				for (auto & p : _output_buffer)
					size += p.size();
				output.resize(size);
				size_t idx = 0;
				for (auto & p : _output_buffer)
				{
					memcpy(output.data() + idx, p.data(), p.size());
					idx += p.size();
				}
			}

			return true;
		}

		bool error_to_ec(const std::vector<char> & payload, ErrorCollector & ec)
		{
			if (payload.size())
			{
				rapidjson::Document doc;
				std::vector<char> _payload(payload.size() + 1);
				memcpy(_payload.data(), payload.data(), payload.size());
				if (doc.ParseInsitu(_payload.data()).HasParseError())
				{
					auto err = ec.add(-1, "JSON parse error (" + std::to_string(doc.GetParseError()) + ")");
					SAS_LOG_ERROR(_logger, err);
					return false;
				}
				if (!doc.HasMember("errors"))
				{
					auto err = ec.add(-1, "member 'errors' is not found");
					SAS_LOG_ERROR(_logger, err);
					return false;
				}
				auto & v = doc["errors"];
				if (!v.IsArray())
				{
					auto err = ec.add(-1, "member 'errors' is not array");
					SAS_LOG_ERROR(_logger, err);
					return false;
				}
				bool has_error(false);
				for (auto it = v.Begin(); it != v.End(); ++it)
				{
					if (!it->IsObject())
					{
						auto err = ec.add(-1, "member 'errors' has invalid element");
						SAS_LOG_ERROR(_logger, err);
						has_error = true;
						continue;
					}
					ec.add(
						it->HasMember("error_code") ? (*it)["error_code"].GetInt() : -1,
						it->HasMember("error_text") ? (*it)["error_text"].GetString() : "(null)");
				}
				return !has_error;
			}

			auto err = ec.add(-1, "no error info");
			SAS_LOG_WARN(_logger, err);
			return true;
		}

	};

	class HTTPConnection : public Connection, public HTTPCaller
	{
		HTTPConnectionOptions _options;
		Logging::LoggerPtr _logger;
		std::string _invoker;
		std::string _module;
		SessionID _session_id;

	public:
		HTTPConnection(const HTTPConnectionOptions & options, const std::string & module, const std::string & invoker) : Connection(), HTTPCaller(module, invoker),
			_options(options),
			_logger(Logging::getLogger("SAS.HTTPConnection." + module + "." + invoker)),
			_invoker(invoker),
			_module(module),
			_session_id(0)
		{ }


		virtual ~HTTPConnection()
		{
			SAS_LOG_NDC();
			NullEC ec;
			endSession(ec);
		}

		bool init(HTTPConnectionOptions & options, ErrorCollector & ec)
		{
			SAS_LOG_NDC();
			if (!HTTPCaller::init(options, ec))
				return false;
			return true;
		}

		virtual bool getSession(ErrorCollector & ec) override
		{
			SAS_LOG_NDC();

			std::vector<char> output;
			Invoker::Status status;
			if (!msg_exchange(_options.method_control, _session_id, std::string(), "get_session", std::vector<char>(), output, status, ec))
				return false;
			
			switch(status)
			{
			case Invoker::Status::OK:
				break;
			case Invoker::Status::NotImplemented:
				{
					auto err = ec.add(-1, "unexpected: mode 'get_session' is not supported");
					SAS_LOG_ERROR(_logger, err);
				}
				return false;
			case Invoker::Status::Error:
			case Invoker::Status::FatalError:
				error_to_ec(output, ec);
				return false;
			}

			return true;
		}

		virtual Status invoke(const std::vector<char> & input, std::vector<char> & output, ErrorCollector & ec) final
		{
			SAS_LOG_NDC();
			
			std::string out_topic;
			std::vector<std::string> out_args;

			Status status;

			if (!msg_exchange(_options.method_invoke, _session_id, _invoker, "invoke", input, output, status, ec))
				return Status::Error;

			if(status != Status::OK)
				error_to_ec(output, ec);

			return status;
		}
		
	private:
		bool endSession(ErrorCollector & ec)
		{
			std::vector<char> output;
			Status status;
			if (!msg_exchange(_options.method_control, _session_id, std::string(), "end_session", std::vector<char>(), output, status, ec))
				return false;

			switch(status)
			{
			case Invoker::Status::OK:
				break;
			case Invoker::Status::NotImplemented:
				{
					auto err = ec.add(-1, "unexpected: mode 'get_session' is not supported");
					SAS_LOG_ERROR(_logger, err);
				}
				return false;
			case Invoker::Status::Error:
			case Invoker::Status::FatalError:
				error_to_ec(output, ec);
				return false;
			}

			_session_id = 0;

			return true;
		}
	};


	struct HTTPConnector::Priv
	{
		Priv(const std::string & name_, Application * app_) :
			name(name_),
			app(app_),
			logger(Logging::getLogger("SAS.HTTPConnector." + name_))
		{ }

		std::string name;
		Application * app;
		Logging::LoggerPtr logger;

		long disconnect_timeout = 0;
		HTTPConnectionOptions options;
	};

	HTTPConnector::HTTPConnector(const std::string & name, Application * app) : Connector(),
		priv(new Priv(name, app))
	{ }

	HTTPConnector::~HTTPConnector()
	{
		delete priv;
	}

	std::string HTTPConnector::name() const
	{
		return priv->name;
	}

	bool HTTPConnector::init(const std::string & path, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		if(!priv->options.build(path, priv->app->configReader(), ec))
			return false;

		return true;
	}

	bool HTTPConnector::connect(ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		// nothing to do
		return true;
	}

	Connection * HTTPConnector::createConnection(const std::string & module_name, const std::string & invoker_name, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		auto conn = new HTTPConnection(priv->options, module_name, invoker_name);
		if (!conn->init(priv->options, ec) || !conn->connect(ec))
		{
			delete conn;
			return nullptr;
		}
		return conn;
	}

	bool HTTPConnector::getModuleInfo(const std::string & moduleName, std::string & description, std::string & version, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		SAS_LOG_VAR(priv->logger, moduleName);
		HTTPCaller caller(moduleName, priv->name);
		if (!caller.init(priv->options, ec))
			return false;
		std::vector<char> output;
		SAS_LOG_TRACE(priv->logger, "caller.msg_exchange");
		Invoker::Status status;
		SessionID _tmp_sid;
		if (!caller.msg_exchange(priv->options.method_control, _tmp_sid, std::string(), "get_module_info", std::vector<char>(), output, status, ec))
			return false;

		switch(status)
		{
		case Invoker::Status::OK:
			break;
		case Invoker::Status::NotImplemented:
			{
				auto err = ec.add(-1, "unexpected: mode 'get_session' is not supported");
				SAS_LOG_ERROR(priv->logger, err);
			}
			return false;
		case Invoker::Status::Error:
		case Invoker::Status::FatalError:
			caller.error_to_ec(output, ec);
			return false;
		}

		rapidjson::Document doc;
		std::vector<char> _payload(output.size() + 1);
		memcpy(_payload.data(), output.data(), output.size());
		if (doc.ParseInsitu(_payload.data()).HasParseError())
		{
			auto err = ec.add(-1, "JSON parse error (" + std::to_string(doc.GetParseError()) + ")");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}
		description = doc.HasMember("description") ? doc["description"].GetString() : "(null)";
		SAS_LOG_VAR(priv->logger, description);
		version = doc.HasMember("version") ? doc["version"].GetString() : "(null)";
		SAS_LOG_VAR(priv->logger, version);
		return true;
	}

}
