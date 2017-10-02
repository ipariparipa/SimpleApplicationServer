/*
This file is part of sasMQTT.

sasMQTT is free software: you can redistribute it and/or modify
it under the terms of the Lesser GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

sasMQTT is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with sasMQTT.  If not, see <http://www.gnu.org/licenses/>
*/

#include "mqttinterface.h"
#include <sasCore/logging.h>
#include <sasCore/errorcollector.h>
#include <sasJSON/jsonerrorcollector.h>
#include <sasCore/module.h>
#include <sasCore/application.h>
#include <sasCore/objectregistry.h>
#include <sasCore/tools.h>
#include <sasCore/configreader.h>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"

#include "threading.h"
#include "mqttconnectionoptions.h"
#include "mqttclient.h"
#include "mqttasync.h"

#include <list>
#include <deque>

namespace SAS {

	class MQTTRunner : public MQTTAsync
	{
		Logging::LoggerPtr logger;
		Application * app;
		std::string name;
	public:
		MQTTRunner(Application * app_, const std::string & name_) :
			MQTTAsync(name_),
			logger(Logging::getLogger("MQTTRunner." + name_)),
			app(app_),
			name(name_),
			threadPool(app, logger)
		{ }

		 virtual ~MQTTRunner() { }

	private:
		struct RunnerTask
		{
			 MQTTAsync * mqtt;
			 std::string topic;
			 std::vector<char> payload;
		};

		class RunnerThread : public ThreadInPool<RunnerTask>
		{
			Application * _app;
			Logging::LoggerPtr _logger;
		public:
			 RunnerThread(Application * app, const Logging::LoggerPtr & logger) :
				 ThreadInPool(), _app(app), _logger(logger)
			{ }
		protected:
			virtual bool complete(RunnerTask * task) override
			{
				SAS_LOG_NDC();
				SAS_LOG_ASSERT(_logger, task, "the 'task' cannot be null");

				try
				{
					std::vector<char> output;

					SAS_LOG_VAR(_logger, task->topic);

					rapidjson::Document out_doc;
					out_doc.Parse("{}");
					JSONErrorCollector ec(out_doc.GetAllocator());

					auto lst = str_split(task->topic, '/');
					if (lst.size() < 3)
					{
						auto err = ec.add(-1, "unknown topic: '" + task->topic + "'");
						SAS_LOG_ERROR(_logger, err);
						return false;
					}

					std::string module, func, msg_id;

					std::vector<std::string> args;
					size_t i(0);
					for (auto & t : lst)
					{
						switch (i++)
						{
						case 0:
							module = t;
							break;
						case 1:
							func = t;
							break;
						case 2:
							msg_id = t;
							break;
						default:
							args.push_back(t);
							break;
						}
					}

					enum OutType
					{
						Out_OK, Out_JSon, Out_Error
					} outType;

					std::vector<std::string> resp_args;
					std::string resp_res;
					std::vector<char> resp_payload;

					if (func == "get_session")
					{
						if (args.size() < 1)
						{
							auto err = ec.add(-1, "invalid argument size");
							SAS_LOG_ERROR(_logger, err);
							resp_res = "error";
							outType = Out_Error;
						}
						else
						{
							Module * _module;
							if (!(_module = _app->objectRegistry()->getObject<Module>(SAS_OBJECT_TYPE__MODULE, module, ec)))
								outType = Out_Error;
							else
							{
								SessionID sid = std::stoull(args[0]);
								Session * session;
								if (!(session = _module->getSession(sid, ec)))
								{
									resp_res = "error";
									outType = Out_Error;
								}
								else
								{
									resp_args.resize(1);
									resp_args[0] = std::to_string(session->id());
									resp_res = "ok";
									outType = Out_OK;
								}
							}
						}
					}
					else if (func == "end_session")
					{
						if (args.size() < 1)
						{
							auto err = ec.add(-1, "invalid argument size");
							SAS_LOG_ERROR(_logger, err);
							outType = Out_Error;
						}
						else
						{
							Module * _module;
							if (!(_module = _app->objectRegistry()->getObject<Module>(SAS_OBJECT_TYPE__MODULE, module, ec)))
							{
								resp_res = "error";
								outType = Out_Error;
							}
							else
							{
								SessionID sid = std::stoull(args[0]);
								_module->endSession(sid);
								resp_res = "ok";
								outType = Out_OK;
							}
						}
					}
					else if (func == "get_module_info")
					{
						Module * _module;
						if (!(_module = _app->objectRegistry()->getObject<Module>(SAS_OBJECT_TYPE__MODULE, module, ec)))
						{
							resp_res = "error";
							outType = Out_Error;
						}
						else
						{
							out_doc.AddMember("description", rapidjson::StringRef(_module->description().c_str()), out_doc.GetAllocator());
							out_doc.AddMember("version", rapidjson::StringRef(_module->version().c_str()), out_doc.GetAllocator());
							resp_res = "result";
							outType = Out_JSon;
						}
					}
					else if (func == "invoke")
					{
						if (args.size() < 2)
						{
							auto err = ec.add(-1, "invalid argument size");
							SAS_LOG_ERROR(_logger, err);
							resp_res = "error";
							outType = Out_Error;
						}
						else
						{
							Module * _module;
							if (!(_module = _app->objectRegistry()->getObject<Module>(SAS_OBJECT_TYPE__MODULE, module, ec)))
								outType = Out_Error;
							else
							{
								SessionID sid = std::stoull(args[0]);
								Session * session;
								if (!(session = _module->getSession(sid, ec)))
								{
									resp_res = "error";
									outType = Out_Error;
								}
								else
								{
									out_doc.AddMember("session_id", session->id(), out_doc.GetAllocator());
									resp_res = "result";
									outType = Out_JSon;
								}

								switch (session->invoke(args[1], task->payload, resp_payload, ec))
								{
								case Invoker::Status::FatalError:
									resp_res = "fatal";
									outType = Out_Error;
									break;
								case Invoker::Status::Error:
									resp_res = "error";
									outType = Out_Error;
									break;
								case Invoker::Status::NotImplemented:
									resp_res = "not_implemented";
									outType = Out_Error;
									break;
								case Invoker::Status::OK:
									resp_res = "ok";
									resp_args.push_back(std::to_string(session->id()));
									outType = Out_OK;
									break;
								}
							}
						}
					}
					else
					{
						auto err = ec.add(-1, "unsupported topic: '" + task->topic + "'");
						SAS_LOG_ERROR(_logger, err);
						outType = Out_Error;
					}

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
							resp_payload.resize(sb.GetSize()+1);
							memcpy(resp_payload.data(), sb.GetString(), sb.GetSize());
						}
						break;
					}

					std::string resp_topic;
					resp_topic = "sas/response/" + msg_id + "/" + resp_res;
					for (auto & a : resp_args)
						resp_topic += "/" + a;
					NullEC ec2;
					resp_payload.push_back('\0');
					return task->mqtt->send(resp_topic, resp_payload, SAS_MQTT__QOS, ec2);
				}
				catch(std::exception & e)
				{
					SAS_LOG_ERROR(_logger, e.what());
					return false;
				}
				catch(...)
				{
					SAS_LOG_FATAL(_logger, "unknown exception");
					return false;
				}
			}
		};

		class RunnerThreadPool : public AbstractThreadPool<RunnerThread>
		{
			Application * _app;
			Logging::LoggerPtr _logger;
		public:
			RunnerThreadPool(Application * app, const Logging::LoggerPtr & logger) :
				 AbstractThreadPool(), _app(app), _logger(logger)
			{ }
		protected:
			virtual std::shared_ptr<RunnerThread> newThread()
			{
				return std::make_shared<RunnerThread>(_app, _logger);
			}
		} threadPool;

	protected:
		virtual bool messageArrived(const std::string & topic, const std::vector<char> & payload, int qus) override
		{
			auto th = threadPool.get();
			auto task = new RunnerTask;
			task->mqtt = this;
			task->payload = payload;
			task->topic = topic;
			th->add(task);
			return true;
		}
	public:
		Interface::Status run(ErrorCollector & ec)
		{
			SAS_LOG_NDC();

			auto mods = app->objectRegistry()->getObjects(SAS_OBJECT_TYPE__MODULE, ec);
			std::vector<std::string> topics(mods.size());
			for (int i(0), l(mods.size()); i < l; ++i)
				topics[i] = mods[i]->name() + "/#";

			if(!subscribe(topics, SAS_MQTT__QOS, ec))
				return MQTTInterface::Status::CannotStart;

			return MQTTAsync::run(ec) ? MQTTInterface::Status::Ended : MQTTInterface::Status::Crashed;

		}

		Interface::Status shutdown(ErrorCollector & ec)
		{
			SAS_LOG_NDC();

			return MQTTAsync::shutdown(ec) ? Interface::Status::Stopped : Interface::Status::CannotStop;
		}


	};


	struct MQTTInterface::Priv
	{
		Priv(const std::string & name_, Application * app_) :
		logger(Logging::getLogger("SAS.MQTTInterface." + name_)),
		app(app_),
		name(name_),
		runner(std::make_unique<MQTTRunner>(app_, name_))
		{ }

		Logging::LoggerPtr logger;
		Application * app;
		std::string name;
		std::unique_ptr<MQTTRunner> runner;
	};

	MQTTInterface::MQTTInterface(const std::string & name, Application * app) :
			SAS::Interface(), priv(new Priv(name, app))
	{ }

	MQTTInterface::~MQTTInterface()
	{
		delete priv;
	}

	//virtual 
	std::string MQTTInterface::name() const //final
	{
		return priv->name;
	}

	//virtual 
	MQTTInterface::Status MQTTInterface::run(ErrorCollector & ec) //final
	{
		SAS_LOG_NDC();
		return priv->runner->run(ec);
	}

	//virtual
	MQTTInterface::Status MQTTInterface::shutdown(ErrorCollector & ec) //final
	{
		SAS_LOG_NDC();
		return priv->runner->shutdown(ec);
	}

	bool MQTTInterface::init(const std::string & config_path, ErrorCollector & ec)
	{
		SAS_LOG_NDC();

		MQTTConnectionOptions options;
		options.clientId = priv->name;

		if(!options.build(config_path, priv->app->configReader(), ec))
			return false;

		return priv->runner->init(options, ec);
	}

	Logging::LoggerPtr MQTTInterface::logger() const
	{
		return priv->logger;
	}

}
