/*
 * mqttasync.cpp
 *
 *  Created on: 2017.09.19.
 *      Author: apps
 */

#include "mqttasync.h"

#include <sasCore/errorcollector.h>
#include <sasCore/logging.h>

#include <MQTTAsync.h>
#include <string.h>

#include <mutex>

namespace SAS {

struct MQTTAsync_priv
{
	MQTTAsync_priv(MQTTAsync * that_, const std::string & name) :
		that(that_),
		mqtt_handle(NULL),
		logger(Logging::getLogger("MQTTAsync." + name))
	{ }

	MQTTAsync * that;
	::MQTTAsync mqtt_handle;
	Logging::LoggerPtr logger;
	MQTTConnectionOptions options;
	std::vector<std::string> topics;
	int t_qos = 0;
	ErrorCollector * runner_ec = nullptr;

	std::mutex runner_mutex;
	std::mutex conn_mut;

	bool subscribe(ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		if(!topics.size())
		{
			SAS_LOG_DEBUG(logger, "nothing to do.");
			return true;
		}
		int _count = topics.size();
		std::vector<char*> _topic(_count);
		std::vector<int> _qos(_count);
		for(int i = 0; i < _count; ++i)
		{
			_topic[i] = (char *)topics[i].c_str();
			_qos[i] = t_qos;
		}

		int rc;
		if((rc = MQTTAsync_subscribeMany(mqtt_handle, _count, &_topic[0], _qos.data(), NULL) != MQTTASYNC_SUCCESS))
		{
			auto err = ec.add(-1, "could not subscribe for MQTT topics ("+std::to_string(rc)+")");
			SAS_LOG_ERROR(logger, err);
			return false;
			}
		return true;
	}

	bool connect(ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		conn_mut.lock();
		MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
		conn_opts.keepAliveInterval = options.keepalive;
		conn_opts.automaticReconnect = 1;
		conn_opts.cleansession = (int)options.cleanSession;
		conn_opts.username = options.username.c_str();
		conn_opts.password = options.password.c_str();
		conn_opts.context = this;
		conn_opts.onSuccess = MQTTAsync_priv::_onConnected;
		conn_opts.onFailure = MQTTAsync_priv::_onConnectionFailed;
		conn_opts.maxInflight = 100;
		int rc;
		if ((rc = MQTTAsync_connect(mqtt_handle, &conn_opts)) != MQTTASYNC_SUCCESS)
		{
			auto err = ec.add(-1, "could not connect to MQTT server (" + std::to_string(rc) + ")");
			SAS_LOG_ERROR(logger, err);
			return false;
		}

		return true;
	}

	static void _connectionLost(void* context, char* cause)
	{
		SAS_LOG_NDC();
		SAS_LOG_ERROR(((MQTTAsync_priv*)context)->logger, "MQTT connection lost: '" + std::string(cause) + "'");
		NullEC ec;
		if(!((MQTTAsync_priv*)context)->that->connect(((MQTTAsync_priv*)context)->runner_ec ? *((MQTTAsync_priv*)context)->runner_ec : ec))
			((MQTTAsync_priv*)context)->runner_mutex.unlock();
	}

	static void _connected(void* context, char* cause)
	{
		SAS_LOG_NDC();
		SAS_LOG_DEBUG(((MQTTAsync_priv*)context)->logger, "MQTT connected: '" + std::string(cause) + "'");
	}

	static int _messageArrived(void* context, char* topicName, int topicLen, MQTTAsync_message* message)
	{
		SAS_LOG_NDC();
		SAS_LOG_ASSERT(((MQTTAsync_priv*)context)->logger, message, "'message' must be not NULL");

		std::string _topic;
		_topic.append((const char *) topicName, topicLen);
		std::vector<char> _payload(message->payloadlen);
		memcpy(_payload.data(), message->payload, message->payloadlen);
		bool ret = ((MQTTAsync_priv*)context)->that->messageArrived(_topic, _payload, message->qos);

		MQTTAsync_freeMessage(&message);
		MQTTAsync_free(topicName);

		return (int)ret;
	}

	static void _deliveryComplete(void* context, MQTTAsync_token token)
	{
		SAS_LOG_NDC();
	}

	static void _onConnected(void* context, MQTTAsync_successData* response)
	{
		SAS_LOG_NDC();
		NullEC ec;
		((MQTTAsync_priv*)context)->conn_mut.unlock();
		((MQTTAsync_priv*)context)->subscribe(((MQTTAsync_priv*)context)->runner_ec ? *((MQTTAsync_priv*)context)->runner_ec : ec);
	}

	static void _onConnectionFailed(void* context, MQTTAsync_failureData* response)
	{
		SAS_LOG_NDC();
		NullEC ec;
		auto err = (((MQTTAsync_priv*)context)->runner_ec ? ((MQTTAsync_priv*)context)->runner_ec : &ec)->add(-1, "connection failed: '"+ std::string(response->message) +"'");
		SAS_LOG_ERROR(((MQTTAsync_priv*)context)->logger, err);
		((MQTTAsync_priv*)context)->conn_mut.unlock();
	}

};

MQTTAsync::MQTTAsync(const std::string & name) : priv(std::make_unique<MQTTAsync_priv>(this, name))
{ }

MQTTAsync::~MQTTAsync()
{ }


//static
void MQTTAsync::globalInit()
{
//	MQTTAsync_init_options opts = MQTTAsync_init_options_initializer;
//	MQTTAsync_global_init(&opts);
}

bool MQTTAsync::init(const MQTTConnectionOptions & conn_opts, ErrorCollector & ec)
{
	SAS_LOG_NDC();

	int rc;
	if((rc = MQTTAsync_create(&priv->mqtt_handle, conn_opts.serverUri.c_str(), conn_opts.clientId.c_str(), MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTASYNC_SUCCESS)
	{
		auto err = ec.add(-1, "could not initialize MQTT ("+std::to_string(rc)+")");
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}

	if ((rc = MQTTAsync_setCallbacks(priv->mqtt_handle, priv.get(), MQTTAsync_priv::_connectionLost, MQTTAsync_priv::_messageArrived, MQTTAsync_priv::_deliveryComplete)) != MQTTASYNC_SUCCESS ||
	    (rc = MQTTAsync_setConnected(priv->mqtt_handle, priv.get(), MQTTAsync_priv::_connected)) != MQTTASYNC_SUCCESS)
	{
		auto err = ec.add(-1, "unexpected error could not set MQTT callbacks ("+std::to_string(rc)+")");
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}

	priv->options = conn_opts;

	return true;
}

bool MQTTAsync::connect(ErrorCollector & ec)
{
	SAS_LOG_NDC();
	return priv->connect(ec);
}

bool MQTTAsync::disconnect(ErrorCollector & ec)
{
	SAS_LOG_NDC();

	int rc;
	if((rc = MQTTAsync_disconnect(priv->mqtt_handle, NULL)) != MQTTASYNC_SUCCESS)
	{
		auto err = ec.add(-1, "could not disconnect from MQTT server (" + std::to_string(rc) + ")");
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}
	return true;
}

bool MQTTAsync::subscribe(const std::vector<std::string> & topics, int qos, ErrorCollector & ec)
{
	SAS_LOG_NDC();

	if(!MQTTAsync_isConnected(priv->mqtt_handle) && !connect(ec))
		return false;

	std::unique_lock<std::mutex> __locker(priv->conn_mut);

	priv->topics = topics;
	priv->t_qos = qos;

	return priv->subscribe(ec);
}

bool MQTTAsync::unsubscribe(ErrorCollector & ec)
{
	SAS_LOG_NDC();
	int _count = priv->topics.size();
	std::vector<char*> _topic(_count);
	for(int i = 0; i < _count; ++i)
		_topic[i] = (char *)priv->topics[i].c_str();

	int rc;
	if((rc = MQTTAsync_unsubscribeMany(priv->mqtt_handle, _count, &_topic[0], NULL) != MQTTASYNC_SUCCESS))
	{
		auto err = ec.add(-1, "could not cancel subscribtion for MQTT topics ("+std::to_string(rc)+")");
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}

	priv->topics.clear();
	priv->t_qos = 0;

	return true;
}

bool MQTTAsync::send(const std::string & topic, const std::vector<char> & payload, int qos, ErrorCollector & ec)
{
	SAS_LOG_NDC();

	MQTTAsync_responseOptions resp = MQTTAsync_responseOptions_initializer;

	int rc;
	if((rc = MQTTAsync_send(priv->mqtt_handle, topic.c_str(), payload.size()-1, (void*)payload.data(), qos, 0, &resp)) != MQTTASYNC_SUCCESS)
	{
		auto err = ec.add(-1, "could not send MQTT message ("+std::to_string(rc)+")");
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}
	if (priv->options.publish_timeout > 0)
		if ((rc = MQTTAsync_waitForCompletion(priv->mqtt_handle, resp.token, priv->options.publish_timeout) != MQTTASYNC_SUCCESS))
		{
			auto err = ec.add(-1, "MQTT message is lost (" + std::to_string(rc) + ")");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}

	return true;
}

bool MQTTAsync::run(ErrorCollector & ec)
{
	priv->runner_ec = &ec;
	priv->runner_mutex.lock();
	priv->runner_mutex.lock();
	priv->runner_ec = nullptr;
	priv->runner_mutex.unlock();
	return true;
}

bool MQTTAsync::shutdown(ErrorCollector & ec)
{
	priv->runner_mutex.unlock();
	return true;
}

const MQTTConnectionOptions & MQTTAsync::connectOptions() const
{
	return priv->options;
}

}
