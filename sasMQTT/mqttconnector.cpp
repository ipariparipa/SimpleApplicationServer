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

#include "mqttconnector.h"

#include <sasCore/logging.h>
#include <sasCore/application.h>
#include <sasCore/thread.h>
#include <sasCore/tools.h>
#include <sasCore/configreader.h>

#include "mqttclient.h"
#include "mqttconnectionoptions.h"

#include <rapidjson/document.h>

#include <sstream>
#include <mutex>

namespace SAS {

	class MQTTCaller
	{
		Logging::LoggerPtr _logger;
		std::string _module;
		std::string _clientId;
		MQTTClient _client;
		std::mutex mut;
		long _rec_count = 0;
	public:
		MQTTCaller(const std::string & module, const std::string & name) :
			_logger(Logging::getLogger("SAS.MQTTCaller." + module + "." + name)),
			_module(module),
			_client(name)
		{ }

		~MQTTCaller()
		{
			_client.deinit();
		}

		bool init(const MQTTConnectionOptions & options, ErrorCollector & ec)
		{
			SAS_LOG_NDC();
			std::unique_lock<std::mutex> __locker(mut);
			if (!_client.init(options, ec))
				return false;
			_clientId = options.clientId;
			return _client.connect(ec);
		}

		bool connect(ErrorCollector & ec)
		{
			return _client.connect(ec);
		}

		bool disconnect(long timeout, ErrorCollector & ec)
		{
			return _client.disconnect(timeout * 1000, ec);
		}

		void deinit()
		{
			std::unique_lock<std::mutex> __locker(mut);
			_client.deinit();
		}

		bool msg_exchange(const std::string & topic, const std::vector<std::string> & arguments, const std::vector<char> & input, std::string & out_topic, std::vector<std::string> & out_arguments, std::vector<char> & output, long receive_count, ErrorCollector & ec)
		{
			SAS_LOG_NDC();
			std::unique_lock<std::mutex> __locker(mut);
			std::stringstream ss;
			ss << Thread::getThreadId();

			std::string msg_id = this->_clientId + "_" + std::to_string((unsigned long) this) + "_" + ss.str();
			std::string send_topic = _module + "/" + topic + "/" + msg_id;
			for (auto & a : arguments)
				send_topic += "/" + a;

			std::vector<char> _input(input.size()+1);
			std::copy(input.begin(), input.end(), _input.begin());

			std::vector<std::string> rec_subs_topic(1);
			rec_subs_topic[0] = "sas/response/" + msg_id + "/#";

			std::string rec_topic;

			if (!_client.exchange(send_topic, _input, SAS_MQTT__QOS, rec_subs_topic, SAS_MQTT__QOS, rec_topic, output, receive_count, ec))
				return false;

			auto lst = str_split(rec_topic, '/');
			if (lst.size() < 3)
			{
				auto err = ec.add(-1, "unexpected topic: '" + rec_topic + "'");
				SAS_LOG_ERROR(_logger, err);
				return false;
			}
			std::list<std::string> args;
			size_t i(0);
			for (auto & t : lst)
			{
				switch (i++)
				{
				case 0:
				case 1:
					break;
				case 2:
					if (t != msg_id)
					{
						auto err = ec.add(-1, "unexpected error: caught an invalid message: '" + rec_topic + "'");
						SAS_LOG_ERROR(_logger, err);
						return false;
					}
					break;
				case 3:
					out_topic = t;
					break;
				default:
					args.push_back(t);
				}
			}

			out_arguments.resize(args.size());
			std::copy(args.begin(), args.end(), out_arguments.begin());

			return true;
		}

		bool error_to_ec(const std::vector<char> & payload, ErrorCollector & ec)
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

	};


	class MQTTConnection : public Connection, public MQTTCaller
	{
		Logging::LoggerPtr _logger;
		std::string _invoker;
		std::string _module;
		long long _session_id;
		long _receive_count = 0;
	public:
		MQTTConnection(const std::string & module, const std::string & invoker) : Connection(), MQTTCaller(module, invoker),
			_logger(Logging::getLogger("SAS.MQTTConnection." + module + "." + invoker)),
			_invoker(invoker),
			_module(module),
			_session_id(0)
		{ }


		virtual ~MQTTConnection()
		{
			SAS_LOG_NDC();
			NullEC ec;
			endSession(ec);
		}

		bool init(MQTTConnectionOptions & options, long receive_count, ErrorCollector & ec)
		{
			SAS_LOG_NDC();
			if(!MQTTCaller::init(options, ec))
				return false;
			_receive_count = receive_count;
			return true;
		}

		virtual bool getSession(ErrorCollector & ec) override
		{
			SAS_LOG_NDC();

			std::vector<std::string> in_args(1);
			in_args[0] = std::to_string(_session_id);
			std::string out_topic;
			std::vector<std::string> out_args;
			std::vector<char> output;
			if (!msg_exchange("get_session", in_args, std::vector<char>(), out_topic, out_args, output, _receive_count, ec))
				return false;
			
			if (out_topic == "error" || out_topic == "fatal")
			{
				error_to_ec(output, ec);
				return false;
			}
			else if (out_topic == "ok")
			{
				if (out_args.size() < 1)
				{
					auto err = ec.add(-1, "invalid number of out arguments");
					SAS_LOG_ERROR(_logger, err);
					return false;
				}
				_session_id = std::stoull(out_args[0]);
				return true;
			}
			auto err = ec.add(-1, "invalid out topic: '"+out_topic+"'");
			SAS_LOG_ERROR(_logger, err);
			return false;
		}

		virtual Status invoke(const std::vector<char> & input, std::vector<char> & output, ErrorCollector & ec) final
		{
			SAS_LOG_NDC();
			
			std::vector<std::string> in_args(2);
			in_args[0] = std::to_string(_session_id);
			in_args[1] = _invoker;
			std::string out_topic;
			std::vector<std::string> out_args;
			if (!msg_exchange("invoke", in_args, input, out_topic, out_args, output, _receive_count, ec))
				return Status::Error;

			if (out_topic == "error")
			{
				error_to_ec(output, ec);
				return Status::Error;
			}
			else if (out_topic == "fatal")
			{
				error_to_ec(output, ec);
				return Status::FatalError;
			}
			else if (out_topic == "not_implemented")
			{
				error_to_ec(output, ec);
				return Status::NotImplemented;
			}
			else if (out_topic == "ok")
			{
				if (out_args.size() < 1)
				{
					auto err = ec.add(-1, "invalid number of out arguments");
					SAS_LOG_ERROR(_logger, err);
					return Status::Error;
				}
				_session_id = std::stoull(out_args[0]);
				return Status::OK;
			}
			auto err = ec.add(-1, "invalid out topic: '" + out_topic + "'");
			SAS_LOG_ERROR(_logger, err);
			return Status::Error;
		}
		
	private:
		bool endSession(ErrorCollector & ec)
		{
			std::vector<std::string> in_args(1);
			in_args[0] = std::to_string(_session_id);
			std::string out_topic;
			std::vector<std::string> out_args;
			std::vector<char> output;
			if (!msg_exchange("end_session", in_args, std::vector<char>(), out_topic, out_args, output, _receive_count, ec))
				return false;

			if (out_topic == "error" || out_topic == "fatal")
			{
				error_to_ec(output, ec);
				return false;
			}
			else if (out_topic == "ok")
			{
				_session_id = 0;
				return true;
			}
			auto err = ec.add(-1, "invalid out topic: '" + out_topic + "'");
			SAS_LOG_ERROR(_logger, err);
			return false;
		}
	};


	struct MQTTConnector_priv
	{
		MQTTConnector_priv(const std::string & name_, Application * app_) :
			name(name_),
			app(app_),
			logger(Logging::getLogger("SAS.MQTTConnector." + name_))
		{ }

		std::string name;
		Application * app;
		Logging::LoggerPtr logger;

		long rec_count = 10;
		long disconnect_timeout = 0;
		MQTTConnectionOptions options;
	};

	MQTTConnector::MQTTConnector(const std::string & name, Application * app) : Connector(),
		priv(std::make_unique<MQTTConnector_priv>(name, app))
	{ }

	MQTTConnector::~MQTTConnector()
	{ }

	std::string MQTTConnector::name() const
	{
		return priv->name;
	}

	bool MQTTConnector::init(const std::string & path, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		if(!priv->options.build(path, priv->app->configReader(), ec))
			return false;

		long long ll_tmp;
		if(!priv->app->configReader()->getNumberEntry(path + "/RECEIVE_COUNT", ll_tmp, priv->rec_count, ec))
			return false;
		ll_tmp = priv->rec_count;

		return true;
	}

	bool MQTTConnector::connect(ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		// nothing to do
		return true;
	}

	Connection * MQTTConnector::createConnection(const std::string & module_name, const std::string & invoker_name, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		auto conn = new MQTTConnection(module_name, invoker_name);
		if (!conn->init(priv->options, priv->rec_count, ec) || !conn->connect(ec))
		{
			delete conn;
			return nullptr;
		}
		return conn;
	}

	bool MQTTConnector::getModuleInfo(const std::string & moduleName, std::string & description, std::string & version, ErrorCollector & ec)
	{
		MQTTCaller caller(moduleName, priv->name);
		if (!caller.init(priv->options, ec))
			return false;
		std::vector<std::string> in_args(1);
		in_args[0] = moduleName;
		std::string out_topic;
		std::vector<std::string> out_args;
		std::vector<char> output;
		if (!caller.msg_exchange("get_module_info", in_args, std::vector<char>(), out_topic, out_args, output, priv->rec_count, ec))
			return false;

		if (out_topic == "error" || out_topic == "fatal")
		{
			caller.error_to_ec(output, ec);
			return false;
		}
		else if (out_topic == "result")
		{
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
			version = doc.HasMember("version") ? doc["version"].GetString() : "(null)";
			return true;
		}
		auto err = ec.add(-1, "invalid out topic: '" + out_topic + "'");
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}

}
