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

#include "mqttconnectionoptions.h"

#include <MQTTClient.h>
#include <MQTTClientPersistence.h>

namespace SAS {

	struct MQTTClient_priv
	{
		MQTTClient_priv(const std::string & name_) :
//			name(name_),
			logger(Logging::getLogger("SAS.MQTTClient." + name_)),
			mqtt_handle(NULL)
		{ }

		MQTTClient_priv(const Logging::LoggerPtr & logger_) :
			logger(logger_),
			mqtt_handle(NULL)
		{ }

//		std::string name;

		Logging::LoggerPtr logger;

		MQTTConnectionOptions options;

		::MQTTClient mqtt_handle;
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
		priv->options = options;

		int rc = MQTTClient_create(&priv->mqtt_handle, (options.host + ":" + options.port).c_str(), options.clientId.c_str(), MQTTCLIENT_PERSISTENCE_NONE, NULL);
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
		if (priv->mqtt_handle)
			MQTTClient_destroy(&priv->mqtt_handle);
		priv->mqtt_handle = NULL;
	}

	bool MQTTClient::publish(const std::string & topic, const std::vector<char> & payload, long qus, long timeout, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		MQTTClient_deliveryToken dt;
		int rc;
		if ((rc = MQTTClient_publish(priv->mqtt_handle, topic.c_str(), payload.size(), (void*)payload.data(), qus, 0, &dt)) != MQTTCLIENT_SUCCESS)
		{
			if (!connect(ec))
				return false;
			if ((rc = MQTTClient_publish(priv->mqtt_handle, topic.c_str(), payload.size(), (void*)payload.data(), qus, 0, &dt)) != MQTTCLIENT_SUCCESS)
			{
				auto err = ec.add(-1, "could not send MQTT message (" + std::to_string(rc) + ")");
				SAS_LOG_ERROR(priv->logger, err);
				return false;
			}
		}
		if (timeout > 0)
			if ((rc = MQTTClient_waitForCompletion(priv->mqtt_handle, dt, timeout) != MQTTCLIENT_SUCCESS))
			{
				auto err = ec.add(-1, "MQTT message is lost (" + std::to_string(rc) + ")");
				SAS_LOG_ERROR(priv->logger, err);
				return false;
			}

		return true;
	}

	bool MQTTClient::receive(const std::vector<std::string> & subscribe, long qus, std::string & topic, std::vector<char> & payload, long timeout, long count, ErrorCollector & ec)
	{
		SAS_LOG_NDC();

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
					unsubscribe(NullEC());
			}

			bool subscribe(ErrorCollector & ec)
			{
				SAS_LOG_NDC();
				int rc;
				std::vector<char *> tmp_topic(topic.size());
				std::vector<int> tmp_qus(topic.size());
				for (size_t i(0), l(topic.size()); i < l; ++i)
				{
					tmp_topic[i] = (char *)topic[i].c_str();
					tmp_qus[i] = qus;
				}
				if((rc = MQTTClient_subscribeMany(client, tmp_topic.size(), tmp_topic.data(), tmp_qus.data())) != MQTTCLIENT_SUCCESS)
				{
					auto err = ec.add(-1, "MQTT: could not subscribe for topics (" + std::to_string(rc) + ")");
					SAS_LOG_ERROR(logger, err);
					return false;
				}
				_active = true;
				return true;
			}

			bool unsubscribe(ErrorCollector & ec)
			{
				SAS_LOG_NDC();
				_active = false;
				int rc;
				std::vector<char *> tmp_topic(topic.size());
				for (size_t i(0), l(topic.size()); i < l; ++i)
					tmp_topic[i] = (char *)topic[i].c_str();
				if ((rc = MQTTClient_unsubscribeMany(client, tmp_topic.size(), tmp_topic.data())) != MQTTCLIENT_SUCCESS)
				{
					auto err = ec.add(-1, "could not cancel subscribtion for topic (" + std::to_string(rc) + ")");
					SAS_LOG_ERROR(logger, err);
					return false;
				}
				return true;
			}

		} subscriber(priv->mqtt_handle, subscribe, qus, priv->logger);

		char * _topic;
		int _topic_len;
		MQTTClient_message* message = NULL;
		int rc;

		if (!subscriber.subscribe(ec))
			return false;

		long i = 0;
		while (count < 0 || i < count)
		{
			if (count >= 0)
				++i;
			if (!(rc = MQTTClient_receive(priv->mqtt_handle, &_topic, &_topic_len, &message, timeout)) != MQTTCLIENT_SUCCESS)
			{
				if (!connect(ec))
					return false;
				if (!subscriber.subscribe(ec))
					return false;
				if (!(rc = MQTTClient_receive(priv->mqtt_handle, &_topic, &_topic_len, &message, timeout)) != MQTTCLIENT_SUCCESS)
				{
					auto err = ec.add(-1, "MQTT connection refused (" + std::to_string(rc) + ")");
					SAS_LOG_ERROR(priv->logger, err);
					return false;
				}
			}
			if (message)
			{
				payload.resize(message->payloadlen);
				memcpy(payload.data(), message->payload, message->payloadlen);
				topic.clear();
				topic.append(_topic, _topic_len);
				MQTTClient_freeMessage(&message);
				MQTTClient_free(&topic);
				return subscriber.unsubscribe(ec);
			}
		}
		auto err = ec.add(-1, "could not get MQTT message (" + std::to_string(rc) + ")");
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}

	bool MQTTClient::receive(const std::vector<std::string> & subscribe, long qus, std::string & topic, std::vector<char> & payload, long timeout, ErrorCollector & ec)
	{
		return receive(subscribe, qus, topic, payload, timeout, -1, ec);
	}

	bool MQTTClient::connect(ErrorCollector & ec)
	{
		MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
		conn_opts.keepAliveInterval = priv->options.keepalive;
		conn_opts.reliable = 0;
		conn_opts.cleansession = 1;
		conn_opts.username = priv->options.username.c_str();
		conn_opts.password = priv->options.password.c_str();
		int rc;
		if ((rc = MQTTClient_connect(priv->mqtt_handle, &conn_opts)) != MQTTCLIENT_SUCCESS)
		{
			auto err = ec.add(-1, "could not connect to MQTT server (" + std::to_string(rc) + ")");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}

		return true;
	}

	bool MQTTClient::disconnect(long timeout, ErrorCollector & ec)
	{
		SAS_LOG_NDC();

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
