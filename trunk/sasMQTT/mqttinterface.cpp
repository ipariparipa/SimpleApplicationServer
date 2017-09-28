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

/*
	class MQTTRunner
	{
		SAS_COPY_PROTECTOR(MQTTRunner)

		MQTTInterface * _interface;
		Logging::LoggerPtr _logger;
		Application * _app;

		struct Task
		{
			std::string module;
			std::string topic;
			std::string msg_id;
			std::vector<std::string> arguments;
			std::vector<char> payload;
		};

		typedef Task Response;
		typedef Task Run;

		class ResponserThread : public ThreadInPool<Response>
		{
			MQTTClient _client;
		public:
			ResponserThread(MQTTConnectionOptions & options, const Logging::LoggerPtr & logger) : ThreadInPool<Response>(logger),
				_client(logger)
			{
				SAS_LOG_NDC();
				NullEC ec;
				SAS_LOG_ASSERT(logger, _client.init(options, ec), "could not initialize mqtt client");
			}

			virtual ~ResponserThread() { }

		protected:
			virtual void execute() override
			{
				while (enterContolledSection())
				{
					auto r = _tasks.take();

					if (!r)
					{
						suspend();
						continue;
					}

					SAS_LOG_NDC();

					std::string topic;
					topic = r->module + "/" + r->msg_id + "/" + r->topic;
					for (auto & a : r->arguments)
						topic += "/" + a;
					NullEC ec;
					r->payload.push_back('\0');
					_client.publish(topic, r->payload, 2, ec);
					delete r;
				}
			}
		};

		class RunnerThread : public ThreadInPool<Run>
		{
			Application * _app;
		public:
			RunnerThread(Application * app, const Logging::LoggerPtr & logger, AbstractThreadPool<ResponserThread> & reponser_threadPool) : ThreadInPool<Run>(logger),
				_app(app),
				_reponser_threadPool(reponser_threadPool)
			{ }

			~RunnerThread() { }

		protected:
			virtual void execute() override
			{
				while (enterContolledSection())
				{
					auto r = _tasks.take();

					if (!r)
					{
						suspend();
						continue;
					}

					SAS_LOG_NDC();

					std::vector<char> output;

					SAS_LOG_VAR(_logger, r->topic);

					rapidjson::Document out_doc;
					out_doc.Parse("{}");
					JSONErrorCollector ec(out_doc.GetAllocator());

					auto resp = new Response();

					resp->msg_id = r->msg_id;
					resp->module = r->module;

					enum OutType
					{
						Out_OK, Out_JSon, Out_Error
					} outType;

					if (r->topic == "get_session")
					{
						if (r->arguments.size() < 1)
						{
							auto err = ec.add(-1, "invalid argument size");
							SAS_LOG_ERROR(_logger, err);
							resp->topic = "error";
							outType = Out_Error;
						}
						else
						{
							Module * module;
							if (!(module = _app->objectRegistry()->getObject<Module>(SAS_OBJECT_TYPE__MODULE, r->module, ec)))
								outType = Out_Error;
							else
							{
								SessionID sid = std::stoull(r->arguments[0]);
								Session * session;
								if (!(session = module->getSession(sid, ec)))
								{
									resp->topic = "error";
									outType = Out_Error;
								}
								else
								{
									resp->arguments.resize(1);
									resp->arguments[0] = std::to_string(session->id());
									resp->topic = "ok";
									outType = Out_OK;
								}
							}
						}
					}
					else if (r->topic == "end_session")
					{
						if (r->arguments.size() < 1)
						{
							auto err = ec.add(-1, "invalid argument size");
							SAS_LOG_ERROR(_logger, err);
							outType = Out_Error;
						}
						else
						{
							Module * module;
							if (!(module = _app->objectRegistry()->getObject<Module>(SAS_OBJECT_TYPE__MODULE, r->module, ec)))
							{
								resp->topic = "error";
								outType = Out_Error;
							}
							else
							{
								SessionID sid = std::stoull(r->arguments[0]);
								module->endSession(sid);
								resp->topic = "ok";
								outType = Out_OK;
							}
						}
					}
					else if (r->topic == "get_module_info")
					{
						Module * module;
						if (!(module = _app->objectRegistry()->getObject<Module>(SAS_OBJECT_TYPE__MODULE, r->module, ec)))
						{
							resp->topic = "error";
							outType = Out_Error;
						}
						else
						{
							out_doc.AddMember("description", rapidjson::StringRef(module->description().c_str()), out_doc.GetAllocator());
							out_doc.AddMember("version", rapidjson::StringRef(module->version().c_str()), out_doc.GetAllocator());
							resp->topic = "result";
							outType = Out_JSon;
						}
					}
					else if (r->topic == "invoke")
					{
						if (r->arguments.size() < 2)
						{
							auto err = ec.add(-1, "invalid argument size");
							SAS_LOG_ERROR(_logger, err);
							resp->topic = "error";
							outType = Out_Error;
						}
						else
						{
							Module * module;
							if (!(module = _app->objectRegistry()->getObject<Module>(SAS_OBJECT_TYPE__MODULE, r->module, ec)))
								outType = Out_Error;
							else
							{
								SessionID sid = std::stoull(r->arguments[0]);
								Session * session;
								if (!(session = module->getSession(sid, ec)))
								{
									resp->topic = "error";
									outType = Out_Error;
								}
								else
								{
									out_doc.AddMember("session_id", session->id(), out_doc.GetAllocator());
									resp->topic = "result";
									outType = Out_JSon;
								}

								switch (session->invoke(r->arguments[1], r->payload, resp->payload, ec))
								{
								case Invoker::Status::FatalError:
									resp->topic = "fatal";
									outType = Out_Error;
									break;
								case Invoker::Status::Error:
									resp->topic = "error";
									outType = Out_Error;
									break;
								case Invoker::Status::NotImplemented:
									resp->topic = "not_implemented";
									outType = Out_Error;
									break;
								case Invoker::Status::OK:
									resp->topic = "output";
									outType = Out_OK;
									break;
								}
							}
						}
					}
					else
					{
						auto err = ec.add(-1, "unsupported topic: '" + r->topic + "'");
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
							resp->payload.resize(sb.GetSize()+1);
							memcpy(resp->payload.data(), sb.GetString(), sb.GetSize());
						}
						break;
					}

					delete r;

					_reponser_threadPool.get()->add(resp);
				}
			}

		private:
			AbstractThreadPool<ResponserThread> & _reponser_threadPool;
		};
		
		class RunnerThreadPool : public AbstractThreadPool<RunnerThread>
		{
			Application * _app;
			Logging::LoggerPtr _logger;
		public:
			RunnerThreadPool(Application * app, const Logging::LoggerPtr & logger) : AbstractThreadPool<RunnerThread>(),
				_app(app),
				_logger(logger),
				_reponser_threadPool(logger)
			{}

			void init(const MQTTConnectionOptions & options)
			{
				_reponser_threadPool.init(options);
			}

		protected:
			virtual std::shared_ptr<RunnerThread> newThread() override
			{
				return std::make_shared<RunnerThread>(_app, _logger, _reponser_threadPool);
			}

		private:
			class ResponserThreadPool : public AbstractThreadPool<ResponserThread>
			{
				Logging::LoggerPtr _logger;
				MQTTConnectionOptions _options;
			public:
				ResponserThreadPool(const Logging::LoggerPtr & logger) :
					_logger(logger)
				{ }

				virtual ~ResponserThreadPool() { }

				void init(const MQTTConnectionOptions & options)
				{
					_options = options;
				}

				virtual std::shared_ptr<ResponserThread> newThread()
				{
					return std::make_shared<ResponserThread>(_options, _logger);
				}

			} _reponser_threadPool;

		} _runner_threadPool;

		MQTTClient _client;
		long _receive_count = 0;
	public:
		MQTTRunner(MQTTInterface * interface, Application * app) :
			_interface(interface),
			_logger(Logging::getLogger("SAS.MQTTRunner." + interface->name())),
			_app(app),
			_runner_threadPool(_app, _logger),
			_client(interface->name())
		{ }

		bool init(const MQTTConnectionOptions & options, long receive_count, ErrorCollector & ec)
		{
			SAS_LOG_NDC();
			if (!_client.init(options, ec))
				return false;
			_runner_threadPool.init(options);
			_receive_count = receive_count;
			return true;
		}

		MQTTInterface::Status run(ErrorCollector & ec)
		{
			SAS_LOG_NDC();
			if (!_client.connect(ec))
				return MQTTInterface::Status::CannotStart;

			auto mods = _app->objectRegistry()->getObjects(SAS_OBJECT_TYPE__MODULE, ec);
			std::vector<std::string> topics(mods.size());
			for (int i(0), l(mods.size()); i < l; ++i)
				topics[i] = mods[i]->name() + "/#";

			while (true)
			{
				auto run = new Run();
				std::string rec_topic;

				if (!_client.receive(topics, 2, rec_topic, run->payload, ec))
					return MQTTInterface::Status::Crashed;

				auto lst = str_split(rec_topic, '/');
				if (lst.size() < 3)
				{
					auto err = ec.add(-1, "unknown topic: '" + rec_topic + "'");
					SAS_LOG_ERROR(_logger, err);
					continue;
				}

				std::list<std::string> args;
				size_t i(0);
				for (auto & t : lst)
				{
					switch (i++)
					{
					case 0:
						run->module = t;
						break;
					case 1:
						run->topic = t;
						break;
					case 2:
						run->msg_id = t;
						break;
					default:
						args.push_back(t);
						break;
					}
				}

				run->arguments.resize(args.size());
				std::copy(args.begin(), args.end(), run->arguments.begin());

				_runner_threadPool.get()->add(run);
			}

			return MQTTInterface::Status::Stopped;
		}

	};
*/

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
//		_runner(std::make_unique<MQTTRunner>(this, app))
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
