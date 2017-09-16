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
#include "rapidjson/document.h"
#include "rapidjson/writer.h"

#include "threading.h"
#include "mqttconnectionoptions.h"
#include "mqttclient.h"

#include <list>
#include <deque>

namespace SAS {

	class MQTTRunner
	{
		SAS_COPY_PROTECTOR(MQTTRunner)

		MQTTInterface * _interface;
		Application * _app;
		Logging::LoggerPtr _logger;

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
				NullEC ec;
				_client.init(options, ec);
			}

			virtual ~ResponserThread() { }

		protected:
			virtual void execute() override
			{
				while (enterContolledSection())
				{
					auto r = _tasks.take();

					if (!r)
						suspend();

					SAS_LOG_NDC();

					std::string topic;
					topic = r->module + "/" + r->msg_id + "/" + r->topic;
					for (auto & a : r->arguments)
						topic += "/" + a;
					NullEC ec;
					_client.publish(topic, r->payload, 2, ec);
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
						suspend();

					SAS_LOG_NDC();

					std::vector<char> output;

					SAS_LOG_VAR(_logger, r->topic);

					rapidjson::Document out_doc;
					out_doc.Parse("{}");
					JSONErrorCollector ec(out_doc.GetAllocator());

					auto resp = std::shared_ptr<Response>();

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
								case Invoker::Status::NotImplemented:
									resp->topic = "not_implemented";
									outType = Out_Error;
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
							resp->payload.resize(sb.GetSize());
							memcpy(resp->payload.data(), sb.GetString(), sb.GetSize());
						}
						break;
					}

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
		MQTTRunner(MQTTInterface * interface_, Application * app) :
			_interface(interface_),
			_app(app),
			_logger(Logging::getLogger("SAS.MQTTRunner." + interface_->name())),
			_runner_threadPool(_app, _logger),
			_client(interface_->name())
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
				topics[i] = mods[i]->name() + "/*";

			while (true)
			{
				auto run = std::make_shared<Run>();
				std::string rec_topic;

				if (!_client.receive(topics, 2, rec_topic, run->payload, _receive_count, ec))
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
					switch (i)
					{
					case 0:
						run->module = t;
						break;
					case 1:
						run->topic = t;
						break;
					case 3:
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


	MQTTInterface::MQTTInterface(const std::string & name, Application * app) : SAS::Interface(),
		_app(app),
		_runner(std::make_unique<MQTTRunner>(this, app)),
		_logger(Logging::getLogger("SAS.MQTTInterface." + name)), 
		_name(name)
	{ }

	//virtual 
	std::string MQTTInterface::name() const //final
	{
		return _name;
	}

	//virtual 
	MQTTInterface::Status MQTTInterface::run(ErrorCollector & ec) //final
	{
		SAS_LOG_NDC();
		return _runner->run(ec);
	}

	bool MQTTInterface::init(const std::string & config_path, ErrorCollector & ec)
	{
		SAS_LOG_NDC();

		MQTTConnectionOptions options;
		long timeout = 0;

		//TODO: build connection info

		return _runner->init(options, timeout, ec);
	}

	Logging::LoggerPtr MQTTInterface::logger() const
	{
		return _logger;
	}

}
