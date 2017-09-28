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

#include "mqttclient.h"

#include <sasCore/logging.h>
#include <sasCore/errorcollector.h>
#include <sasCore/thread.h>

#include "mqttconnectionoptions.h"

#include <MQTTClient.h>
#include <MQTTClientPersistence.h>

#include <memory>
#include <mutex>
#include <string.h>

#include <iostream>

namespace SAS {

	struct MQTTClient_priv
	{
		MQTTClient_priv(const std::string & name_) :
			logger(Logging::getLogger("SAS.MQTTClient." + name_)),
			mqtt_handle(NULL)
		{ }

		MQTTClient_priv(const Logging::LoggerPtr & logger_) :
			logger(logger_),
			mqtt_handle(NULL)
		{ }

		Logging::LoggerPtr logger;

		MQTTConnectionOptions options;

		::MQTTClient mqtt_handle;

		std::mutex mut;

		bool _publish(const std::string & topic, const std::vector<char> & payload, long qus, ErrorCollector & ec)
		{
			SAS_LOG_NDC();

			if(!MQTTClient_isConnected(mqtt_handle) && !_connect(ec))
				return false;

			MQTTClient_deliveryToken dt;
			int rc;
			if ((rc = MQTTClient_publish(mqtt_handle, topic.c_str(), payload.size()-1, (void*)payload.data(), qus, 0, &dt)) != MQTTCLIENT_SUCCESS)
			{
				if (!_connect(ec))
					return false;
				if ((rc = MQTTClient_publish(mqtt_handle, topic.c_str(), payload.size()-1, (void*)payload.data(), qus, 0, &dt)) != MQTTCLIENT_SUCCESS)
				{
					auto err = ec.add(-1, "could not send MQTT message (" + std::to_string(rc) + ")");
					SAS_LOG_ERROR(logger, err);
					return false;
				}
			}
			if (options.publish_timeout > 0)
				if ((rc = MQTTClient_waitForCompletion(mqtt_handle, dt, options.publish_timeout) != MQTTCLIENT_SUCCESS))
				{
					auto err = ec.add(-1, "MQTT message is lost (" + std::to_string(rc) + ")");
					SAS_LOG_ERROR(logger, err);
					return false;
				}

			return true;
		}

		bool _connect(ErrorCollector & ec)
		{
			SAS_LOG_NDC();

			MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
			conn_opts.keepAliveInterval = options.keepalive;
			conn_opts.connectTimeout = options.connectTimeout;
			conn_opts.retryInterval = options.retryInterval;
			conn_opts.reliable = 0;
			conn_opts.cleansession = options.cleanSession;
			conn_opts.username = options.username.c_str();
			conn_opts.password = options.password.c_str();
			int rc;
			if ((rc = MQTTClient_connect(mqtt_handle, &conn_opts)) != MQTTCLIENT_SUCCESS)
			{
				auto err = ec.add(-1, "could not connect to MQTT server (" + std::to_string(rc) + ")");
				SAS_LOG_ERROR(logger, err);
				return false;
			}

			return true;
		}

	};

	MQTTClient::MQTTClient(const std::string & name) : priv(std::make_unique<MQTTClient_priv>(name))
	{ }

	MQTTClient::MQTTClient(const Logging::LoggerPtr & logger) : priv(std::make_unique<MQTTClient_priv>(logger))
	{ }

	MQTTClient::~MQTTClient() 
	{
		deinit();
	}

	bool MQTTClient::init(const MQTTConnectionOptions & options, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		std::unique_lock<std::mutex> __locker(priv->mut);
		priv->options = options;

		std::stringstream _client_id;
		_client_id << options.clientId; //<< "_" << Thread::getThreadId();

		int rc = MQTTClient_create(&priv->mqtt_handle, options.serverUri.c_str(), _client_id.str().c_str(), MQTTCLIENT_PERSISTENCE_NONE, NULL);
		if (rc != MQTTCLIENT_SUCCESS)
		{
			auto err = ec.add(-1, "could not create MQTT client (" + std::to_string(rc) + ")");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}
		return true;
	}

	void MQTTClient::deinit()
	{
		SAS_LOG_NDC();
		std::unique_lock<std::mutex> __locker(priv->mut);
		if (priv->mqtt_handle)
			MQTTClient_destroy(&priv->mqtt_handle);
		priv->mqtt_handle = NULL;
	}

	bool MQTTClient::publish(const std::string & topic, const std::vector<char> & payload, long qos, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		std::unique_lock<std::mutex> __locker(priv->mut);
		return priv->_publish(topic, payload, qos, ec);
	}

	bool MQTTClient::receive(const std::vector<std::string> & subscribe, long qus, std::string & topic, std::vector<char> & payload, long count, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		std::unique_lock<std::mutex> __locker(priv->mut);

		return exchange(std::string(), std::vector<char>(), 0, subscribe, qus, topic, payload, count, ec);
	}

	bool MQTTClient::receive(const std::vector<std::string> & subscribe, long qus, std::string & topic, std::vector<char> & payload, ErrorCollector & ec)
	{
		return receive(subscribe, qus, topic, payload, -1, ec);
	}

	bool MQTTClient::exchange(const std::string & in_topic, const std::vector<char> & in_payload, long in_qus,
			const std::vector<std::string> & subscribe, long out_qus, std::string & out_topic, std::vector<char> & out_payload, long count,
			ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		std::unique_lock<std::mutex> __locker(priv->mut);

		class  Subscriber
		{
			::MQTTClient client;
			std::vector<std::string>  topic;
			int qus;
			Logging::LoggerPtr logger;
			bool _active;
		public:
			Subscriber(::MQTTClient client_, const std::vector<std::string> & topic_, int qus_, Logging::LoggerPtr logger_) :
				client(client_),
				topic(topic_),
				qus(qus_),
				logger(logger_),
				_active(false)
			{ }

			~Subscriber()
			{
				if (!_active)
				{
					NullEC ec;
					unsubscribe(ec);
				}
			}

			bool subscribe(ErrorCollector & ec)
			{
				SAS_LOG_NDC();
				if(topic.size())
				{
					int rc;
					std::vector<char *> tmp_topic(topic.size());
					std::vector<int> tmp_qus(topic.size());
					for (size_t i(0), l(topic.size()); i < l; ++i)
					{
						tmp_topic[i] = (char *)topic[i].c_str();
						tmp_qus[i] = qus;
					}
					if((rc = MQTTClient_subscribeMany(client, tmp_topic.size(), &tmp_topic[0], tmp_qus.data())) != MQTTCLIENT_SUCCESS)
					{
						auto err = ec.add(-1, "MQTT: could not subscribe for topics (" + std::to_string(rc) + ")");
						SAS_LOG_ERROR(logger, err);
						return false;
					}
				}
				_active = true;
				return true;
			}

			bool unsubscribe(ErrorCollector & ec)
			{
				SAS_LOG_NDC();
				_active = false;
				if(topic.size())
				{
					int rc;
					std::vector<char *> tmp_topic(topic.size());
					for (size_t i(0), l(topic.size()); i < l; ++i)
						tmp_topic[i] = (char *)topic[i].c_str();

					if (MQTTClient_isConnected(client) && (rc = MQTTClient_unsubscribeMany(client, tmp_topic.size(), &tmp_topic[0])) != MQTTCLIENT_SUCCESS)
					{
						auto err = ec.add(-1, "could not cancel subscribtion for topic (" + std::to_string(rc) + ")");
						SAS_LOG_ERROR(logger, err);
						return false;
					}
				}
				return true;
			}

		} subscriber(priv->mqtt_handle, subscribe, out_qus, priv->logger);

		auto _connect = [&]() -> bool {
			if(!MQTTClient_isConnected(priv->mqtt_handle))
					if(!priv->_connect(ec))
						return false;
			if(!subscriber.subscribe(ec))
			{
				if(!priv->_connect(ec))
					return false;
				if(!subscriber.subscribe(ec))
					return false;
			}
			return true;
		};

		if(!_connect())
			return false;

		if(in_topic.length() && !priv->_publish(in_topic, in_payload, in_qus, ec))
				return false;

		char * _topic;
		int _topic_len;
		MQTTClient_message* message = NULL;
		int rc;

		long i = 0;
		while (count < 0 || i < count)
		{
			if (count >= 0)
				++i;

			if(!_connect())
				return false;

			if ((rc = MQTTClient_receive(priv->mqtt_handle, &_topic, &_topic_len, &message, priv->options.receive_timeout)) != MQTTCLIENT_SUCCESS)
			{
				if(!_connect())
					return false;

				if ((rc = MQTTClient_receive(priv->mqtt_handle, &_topic, &_topic_len, &message, priv->options.receive_timeout)) != MQTTCLIENT_SUCCESS)
				{
					auto err = ec.add(-1, "MQTT connection refused (" + std::to_string(rc) + ")");
					SAS_LOG_ERROR(priv->logger, err);
					return false;
				}
			}
			if (message)
			{
				out_payload.resize(message->payloadlen);
				memcpy(out_payload.data(), message->payload, message->payloadlen);
				assert(_topic);
				out_topic.clear();
				out_topic.append(_topic, _topic_len);
				if(message)
					MQTTClient_freeMessage(&message);
				MQTTClient_free(&_topic);
				return subscriber.unsubscribe(ec);
			}
		}
		auto err = ec.add(-1, "could not get MQTT message (" + std::to_string(rc) + ")");
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}

	bool MQTTClient::connect(ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		std::unique_lock<std::mutex> __locker(priv->mut);
		return priv->_connect(ec);
	}

	bool MQTTClient::disconnect(long timeout, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		std::unique_lock<std::mutex> __locker(priv->mut);

		int rc;
		if (!(rc = MQTTClient_disconnect(&priv->mqtt_handle, timeout)) != MQTTCLIENT_SUCCESS)
		{
			auto err = ec.add(-1, "could not stop connection to MQTT server (" + std::to_string(rc) + ")");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}

		return true;
	}

}
