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

#include "httpinterface.h"

#ifdef SAS_HTTP__HAVE_MICROHTTPD

#include "httpcommon.h"

#include <sasCore/logging.h>
#include <sasCore/errorcollector.h>
#include <sasJSON/jsonerrorcollector.h>
#include <sasCore/module.h>
#include <sasCore/application.h>
#include <sasCore/objectregistry.h>
#include <sasCore/tools.h>
#include <sasCore/configreader.h>
#include <sasCore/thread.h>
#include <sasCore/controlledthread.h>
#include <sasCore/notifier.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include <list>
#include <deque>
#include <mutex>
#include <limits>

#include <microhttpd.h>

namespace SAS {

	struct HTTPInterface::Priv
	{
		Priv(const std::string & name_, Application * app_) :
		logger(Logging::getLogger("SAS.HTTPInterface." + name_)),
		app(app_),
		name(name_)
		{ }

		Logging::LoggerPtr logger;
		Application * app;
		std::string name;

		MHD_Daemon *daemon = nullptr;

		Notifier runner_not;

		struct Options
		{
			short port = 0;
			std::string responseContentType;
		} options;

		struct connection_info_struct
		{
			connection_info_struct(Priv * priv_) : priv(priv_)
			{ }

			Priv * priv;

			std::list<std::vector<char>> in_buffer;

			HTTPMethod connectiontype = HTTPMethod::None;
			MHD_PostProcessor *postprocessor = nullptr;
		};

		int send_data (struct MHD_Connection *connection, const char * data, size_t size, const char * sid, const char * content_type, int status_code)
		{
			SAS_LOG_NDC();

			MHD_Response *response;
			SAS_LOG_TRACE(logger, "MHD_create_response_from_buffer");
			if (!(response = MHD_create_response_from_buffer(size, (void*) data, MHD_RESPMEM_MUST_COPY)))
				return MHD_NO;

			SAS_LOG_TRACE(logger, "MHD_add_response_header");
			MHD_add_response_header(response, "Content-type", content_type ? content_type : "application/octet-stream");
//			SAS_LOG_TRACE(logger, "MHD_add_response_header");
//			MHD_add_response_header(response, "Content-Length", std::to_string(size).c_str());
			if(sid)
			{
				SAS_LOG_TRACE(logger, "MHD_add_response_header");
				MHD_add_response_header(response, "SID", sid);
			}

			SAS_LOG_TRACE(logger, "MHD_queue_response");
			auto ret = MHD_queue_response (connection, status_code, response);
			SAS_LOG_TRACE(logger, "MHD_destroy_response");
			MHD_destroy_response (response);

			return ret;
		}

		int send_data (struct MHD_Connection *connection, const std::vector<char> & data, const char * sid, const char * content_type, int status_code)
		{
			return send_data(connection, data.data(), data.size(), sid, content_type, status_code);
		}

		int send_data(struct MHD_Connection *connection, const std::list<std::vector<char>> & buffer, const char * sid, const char * content_type, int status_code)
		{
			size_t size = 0;
			for(auto & p : buffer)
				size += p.size();

			std::vector<char> _buffer(size);
			size_t idx = 0;
			for(auto & p : buffer)
			{
				memcpy(_buffer.data() + idx, p.data(), p.size());
				idx += p.size();
			}
			return send_data (connection, _buffer, sid, content_type, status_code);
		}

		void handle_input_data(connection_info_struct *con_info, size_t size, const char *data)
		{
			std::vector<char> buff(size);
			memcpy(buff.data(), data, size);
			con_info->in_buffer.push_back(buff);
		}

		int complete(connection_info_struct *con_info, MHD_Connection *connection, const char * url)
		{
			SAS_LOG_NDC();

			rapidjson::Document out_doc;
			out_doc.SetObject();
			JSONErrorCollector ec(out_doc.GetAllocator());

			enum OutType
			{
				Out_OK, Out_JSon, Out_Error
			} outType = Out_OK;

			std::vector<char> output;
			SessionID sid = 0;
			int answercode = MHD_HTTP_OK;

			SAS_LOG_TRACE(logger, "MHD_lookup_connection_value");
			auto _mode = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "mode");
			if(!_mode)
			{
				auto err = ec.add(-1, std::string() + "mode is not specified");
				SAS_LOG_ERROR(logger, err);
				outType = Out_Error;
				answercode = MHD_HTTP_BAD_REQUEST;
			}
			else
			{
				std::string _url(url);
				while (_url.front() == '/')
					_url =_url.substr(1, -1);
				while (_url.back() == '/')
					_url.pop_back();

				std::stringstream ss(_url);
			    std::string item;
			    std::vector<std::string> splittedUrl;
			    while (std::getline(ss, item, '/'))
			    	if(item.length())
			    		splittedUrl.push_back(item);


				auto get_session_id = [&](SessionID & sid) -> bool
				{
					SAS_LOG_TRACE(logger, "MHD_lookup_connection_value");
					auto sid_str = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "SID");
					if(!sid_str)
						if(!(sid_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "sid")))
							if(splittedUrl.size() >= 2)
								sid_str = splittedUrl[1].c_str();

					if(sid_str)
					{
						try
						{
							sid = std::stoull(sid_str);
						}
						catch(std::exception & e)
						{
							auto err = ec.add(-1, std::string() + "could not convert session ID '" + sid_str + "': '" + e.what() + "'" );
							SAS_LOG_ERROR(logger, err);
							return false;
						}
					}
					else
						sid = 0;
					return true;
				};

				auto get_module = [&]() -> Module *
				{
					if(!splittedUrl.size())
					{
						auto err = ec.add(-1, "module name is nor specified in URL");
						SAS_LOG_ERROR(logger, err);
						return nullptr;
					}

					return app->objectRegistry()->getObject<Module>(SAS_OBJECT_TYPE__MODULE, splittedUrl[0], ec);
				};

				std::string mode(_mode);
				if (mode == "invoke")
				{
					Module * module;
					if (!get_session_id(sid) || !(module = get_module()))
					{
						answercode = MHD_HTTP_BAD_REQUEST;
						outType = Out_Error;
					}
					else
					{
						SAS_LOG_TRACE(logger, "MHD_lookup_connection_value");
						auto invoker_name = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "Invoker");
						if (!invoker_name)
							if (!(invoker_name = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "invoker")))
								if(splittedUrl.size() >= 3)
									invoker_name = splittedUrl[2].c_str();

						if (!invoker_name)
						{
							auto err = ec.add(-1, std::string() + "invoker is not specified");
							SAS_LOG_ERROR(logger, err);
							outType = Out_Error;
							answercode = MHD_HTTP_BAD_REQUEST;
						}

						auto session = module->getSession(sid, ec);
						if (!session)
						{
							outType = Out_Error;
							answercode = MHD_HTTP_INTERNAL_SERVER_ERROR;
						}

						if(answercode == MHD_HTTP_OK)
						{
							sid = session->id();
							size_t size = 0;
							for (auto & p : con_info->in_buffer)
								size += p.size();

							std::vector<char> input(size);
							size_t idx = 0;
							for (auto & p : con_info->in_buffer)
							{
								memcpy(input.data() + idx, p.data(), p.size());
								idx += p.size();
							}

							switch (session->invoke(invoker_name, input, output, ec))
							{
							case Invoker::Status::OK:
								outType = Out_OK;
								answercode = MHD_HTTP_OK;
								break;
							case Invoker::Status::NotImplemented:
								outType = Out_Error;
								answercode = MHD_HTTP_NOT_IMPLEMENTED;
								break;
							case Invoker::Status::Error:
							case Invoker::Status::FatalError:
								outType = Out_Error;
								answercode = MHD_HTTP_INTERNAL_SERVER_ERROR;
								break;
							}
							session->unlock();
						}
					}
				}
				else if(mode == "get_session")
				{
					Module * module;
					if (!get_session_id(sid) || !(module = get_module()))
					{
						answercode = MHD_HTTP_BAD_REQUEST;
						outType = Out_Error;
					}
					else
					{
						auto session = module->getSession(sid, ec);
						if (!session)
							outType = Out_Error;
						else
						{
							sid = session->id();
							session->unlock();
							outType = Out_OK;
						}
					}
				}
				else if (mode == "end_session")
				{
					Module * module;
					if(!get_session_id(sid) || !(module = get_module()))
					{
						answercode = MHD_HTTP_BAD_REQUEST;
						outType = Out_Error;
					}
					else
					{
						module->endSession(sid);
						answercode = MHD_HTTP_OK;
						outType = Out_OK;
					}
				}
				else if(mode == "get_module_info")
				{
					Module * module;
					if(!(module = get_module()))
					{
						answercode = MHD_HTTP_BAD_REQUEST;
						outType = Out_Error;
					}
					else
					{
						auto create_string = [](rapidjson::Document & doc, const char * str) -> rapidjson::Value
						{
							rapidjson::Value v(rapidjson::kStringType);
							v.SetString(str, doc.GetAllocator());
							return v;
						};

						out_doc.AddMember("description", create_string(out_doc, module->description().c_str()), out_doc.GetAllocator());
						out_doc.AddMember("version", create_string(out_doc, module->version().c_str()), out_doc.GetAllocator());
						answercode = MHD_HTTP_OK;
						outType = Out_JSon;
					}
				}
				else
				{
					auto err = ec.add(-1, std::string() + "not supported mode '" + mode + "'");
					SAS_LOG_ERROR(logger, err);
					answercode = MHD_HTTP_BAD_REQUEST;
					outType = Out_Error;
				}
			}

			auto content_type = options.responseContentType.c_str();

			switch (outType)
			{
			case Out_OK:
				break;
			case Out_Error:
				out_doc.AddMember("errors", ec.errors(), out_doc.GetAllocator());
				//no break
			case Out_JSon:
				{
					rapidjson::StringBuffer sb;
					rapidjson::Writer<rapidjson::StringBuffer> w(sb);
					out_doc.Accept(w);
					output.resize(sb.GetSize());
					memcpy(output.data(), sb.GetString(), sb.GetSize());
					content_type = "application/json";
				}
				break;
			}

			return send_data(connection, output, sid ? std::to_string(sid).c_str() : nullptr, content_type, answercode);
		}

		static int iterate_post (void *coninfo_cls, enum MHD_ValueKind kind, const char *key, const char *filename, const char *content_type,
					  const char *transfer_encoding, const char *data, uint64_t off, size_t size)
		{
			if (std::string(key) == "invoke")
			{
				connection_info_struct *con_info = (connection_info_struct *)coninfo_cls;
				if (size > 0)
					con_info->priv->handle_input_data(con_info, size, data);
			}

			return MHD_YES;
		}

		static int answer_to_connection (void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version,
					  const char *upload_data, size_t *upload_data_size, void **con_cls)
		{
			SAS_LOG_NDC();

			assert(cls);
			auto priv = (Priv*)cls;

			if (!*con_cls)
			{
				auto con_info = new connection_info_struct(priv);

				if (std::string(method) == MHD_HTTP_METHOD_POST)
				{
					SAS_LOG_TRACE(priv->logger, "MHD_create_post_processor");
					if (!(con_info->postprocessor = MHD_create_post_processor(connection, 1024 /*buffer size*/, iterate_post, (void*) con_info)))
					{
						delete con_info;
						return MHD_NO;
					}
					con_info->connectiontype = HTTPMethod::POST;
				}
				else if (std::string(method) == MHD_HTTP_METHOD_PUT)
					con_info->connectiontype = HTTPMethod::PUT;
				else if (std::string(method) == MHD_HTTP_METHOD_GET)
					con_info->connectiontype = HTTPMethod::GET;

				*con_cls = (void*) con_info;
				return MHD_YES;
			}

			auto con_info = (connection_info_struct *) *con_cls;

			switch(con_info->connectiontype)
			{
			case HTTPMethod::None:
				return MHD_HTTP_INTERNAL_SERVER_ERROR;
			case HTTPMethod::POST:
				if (*upload_data_size)
				{
					if (con_info->postprocessor)
					{
						SAS_LOG_TRACE(priv->logger, "MHD_post_process");
						if (MHD_post_process(con_info->postprocessor, upload_data, *upload_data_size) != MHD_YES)
							return MHD_HTTP_BAD_REQUEST;
					}
					else
						priv->handle_input_data(con_info, *upload_data_size, upload_data);
					*upload_data_size = 0;
					return MHD_YES;
				}
				else
					return priv->complete(con_info, connection, url);
			case HTTPMethod::PUT:
				if(*upload_data_size)
				{
					priv->handle_input_data(con_info, *upload_data_size, upload_data);
					*upload_data_size = 0;
					return MHD_YES;
				}
				else
					return priv->complete(con_info, connection, url);
			case HTTPMethod::GET:
				return priv->complete(con_info, connection, url);
			}

			return priv->send_data(connection, std::vector<char>(), nullptr, nullptr, MHD_HTTP_BAD_REQUEST);
		}

		static void request_completed (void *cls, struct MHD_Connection *connection, void **con_cls, enum MHD_RequestTerminationCode toe)
		{
			SAS_LOG_NDC();

			assert(cls);
			auto priv = (Priv*)cls;

			auto con_info = (connection_info_struct*)*con_cls;
			if (!con_info)
				return;

			if (con_info->connectiontype == HTTPMethod::POST)
			{
				if (con_info->postprocessor)
				{
					SAS_LOG_TRACE(priv->logger, "MHD_destroy_post_processor");
					MHD_destroy_post_processor(con_info->postprocessor);
				}
			}

			delete con_info;
			*con_cls = nullptr;
		}
	};

	HTTPInterface::HTTPInterface(const std::string & name, Application * app) :
			Interface(), priv(new Priv(name, app))
	{ }

	HTTPInterface::~HTTPInterface()
	{
		delete priv;
	}

	//virtual 
	std::string HTTPInterface::name() const //final override
	{
		return priv->name;
	}

	//virtual 
	HTTPInterface::Status HTTPInterface::run(ErrorCollector & ec) //final override
	{
		SAS_LOG_NDC();

		SAS_LOG_TRACE(priv->logger, "MHD_start_daemon");
		if(!(priv->daemon = MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION,
								 priv->options.port, NULL, NULL,
								 &Priv::answer_to_connection, (void*)priv,
								 MHD_OPTION_NOTIFY_COMPLETED, &Priv::request_completed, (void*)priv,
								 MHD_OPTION_END)))
		{
			auto err = ec.add(-1, std::string() + "could not start HTTP daemon");
			SAS_LOG_ERROR(priv->logger, err);
			return Status::CannotStart;
		}

		priv->runner_not.wait();

		return Status::Ended;
	}

	//virtual
	HTTPInterface::Status HTTPInterface::shutdown(ErrorCollector & ec) //final
	{
		SAS_LOG_NDC();

		SAS_LOG_TRACE(priv->logger, "MHD_stop_daemon");
		MHD_stop_daemon(priv->daemon);
		priv->daemon = nullptr;

		priv->runner_not.notify();

		return Status::Stopped;
	}

	bool HTTPInterface::init(const std::string & config_path, ErrorCollector & ec)
	{
		SAS_LOG_NDC();

		long long _ll_tmp;
		if(!priv->app->configReader()->getNumberEntry(config_path + "/PORT", _ll_tmp, 80, ec))
			return false;
		priv->options.port = (short)_ll_tmp;

		if(!priv->app->configReader()->getStringEntry(config_path + "/RESPONSE_CONTENT_TYPE", priv->options.responseContentType, "application/octet-stream", ec))
			return false;

		return true;
	}

	Logging::LoggerPtr HTTPInterface::logger() const
	{
		return priv->logger;
	}

}

#endif // SAS_HTTP__HAVE_MICROHTTPD
