/*
 * mqttasync.cpp
 *
 *  Created on: 2017.09.19.
 *      Author: apps
 */

#include "mqttasync.h"

#include <sasCore/errorcollector.h>
#include <sasCore/logging.h>
#include <sasCore/thread.h>
#include <sasCore/uniqueobjectmanager.h>

#include <MQTTAsync.h>
#include <string.h>

#include <mutex>
#include <condition_variable>

namespace SAS {

struct MQTTAsync::Priv
{
	Priv(MQTTAsync * that_, const std::string & name) :
		that(that_),
		logger(Logging::getLogger("SAS.MQTTAsync." + name))
	{ }

	Priv(MQTTAsync * that_, const Logging::LoggerPtr & logger_) :
		that(that_),
		logger(logger_)
	{ }

	std::string client_id;
	MQTTAsync * that;
	::MQTTAsync mqtt_handle = NULL;
	Logging::LoggerPtr logger;
	MQTTConnectionOptions options;
	std::vector<std::string> topics;
	int t_qos = 0;
	
	std::mutex mut;
	ErrorCollector * ec = nullptr;

	Notifier runner_not;
	Notifier conn_not;
	Notifier disconn_not;

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
		SAS_LOG_TRACE(logger, "MQTTAsync_subscribeMany");
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

		std::unique_lock<std::mutex> __locker(mut);

		MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
		conn_opts.keepAliveInterval = options.keepalive;
		conn_opts.automaticReconnect = 0;
		conn_opts.cleansession = (int)options.cleanSession;
		conn_opts.username = options.username.c_str();
		conn_opts.password = options.password.c_str();
		conn_opts.context = this;
		conn_opts.onSuccess = Priv::_onConnected;
		conn_opts.onFailure = Priv::_onConnectionFailed;
		conn_opts.maxInflight = 100;
		conn_opts.connectTimeout = options.connectTimeout;
		int rc;
		SAS_LOG_TRACE(logger, "MQTTAsync_connect");
		if ((rc = MQTTAsync_connect(mqtt_handle, &conn_opts)) != MQTTASYNC_SUCCESS)
		{
			auto err = ec.add(-1, "could not connect to MQTT server (" + std::to_string(rc) + ")");
			SAS_LOG_ERROR(logger, err);
			return false;
		}

		if (!conn_not.wait(2000 * options.connectTimeout))
			return false;

		return true;
	}

	static void _connectionLost(void* context, char* cause)
	{
		SAS_LOG_NDC();
		auto priv = (Priv*)context;
		SAS_LOG_ERROR(priv->logger, "MQTT connection lost: '" + std::string(cause ? cause : "(no_cause)") + "'");
		NullEC ec;
		if (!priv->that->connect(priv->ec ? *priv->ec : ec))
			priv->runner_not.notify();
	}

	static void _connected(void* context, char* cause)
	{
		SAS_LOG_NDC();
		auto priv = (Priv*)context;
		SAS_LOG_DEBUG(priv->logger, "MQTT connected: '" + std::string(cause ? cause : "(no_cause)") + "'");
		priv->conn_not.notify();
	}

	static int _messageArrived(void* context, char* topicName, int topicLen, MQTTAsync_message* message)
	{
		SAS_LOG_NDC();
		auto priv = (Priv*)context;
		SAS_LOG_ASSERT(priv->logger, message, "'message' must be not NULL");

		std::string _topic;
		_topic.append((const char *) topicName, topicLen);
		std::vector<char> _payload(message->payloadlen);
		memcpy(_payload.data(), message->payload, message->payloadlen);
		bool ret = priv->that->messageArrived(_topic, _payload, message->qos);

		SAS_LOG_TRACE(priv->logger, "MQTTAsync_freeMessage");
		MQTTAsync_freeMessage(&message);
		SAS_LOG_TRACE(priv->logger, "MQTTAsync_free");
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
		auto priv = (Priv*)context;
		SAS_LOG_DEBUG(priv->logger, "MQTT connected");
		priv->subscribe(priv->ec ? *priv->ec : ec);
		priv->conn_not.notify();
	}

	static void _onConnectionFailed(void* context, MQTTAsync_failureData* response)
	{
		SAS_LOG_NDC();
		NullEC ec;
		auto priv = (Priv*)context;
		auto err = (priv->ec ? priv->ec : &ec)->add(-1, "connection failed: '" + std::string(response->message ? response->message : "(no_message)") + "'");
		SAS_LOG_ERROR(priv->logger, err);
		priv->conn_not.notify();
	}

	static void _onDisconnected(void* context, MQTTAsync_successData* response)
	{
		SAS_LOG_NDC();
		NullEC ec;
		auto priv = (Priv*)context;
		SAS_LOG_DEBUG(priv->logger, "MQTT disconnected");
		priv->disconn_not.notify();
	}

	static void _onDisconnectFailed(void * context, MQTTAsync_failureData* response)
	{
		SAS_LOG_NDC();
		NullEC ec;
		auto priv = (Priv*)context;
		auto err = (priv->ec ? priv->ec : &ec)->add(-1, "connection failed: '" + std::string(response->message ? response->message : "(no_message)") + "'");
		SAS_LOG_ERROR(priv->logger, err);
		priv->disconn_not.notify();
	}

	struct UniqueIdProvider
	{
		~UniqueIdProvider()
		{
			if (id)
				uom.unuse(id);
		}

		UniqueId get(ErrorCollector & ec)
		{
			return id ? id : (id = uom.getUniqueId(ec));
		}
	private:
		static UniqueObjectManager uom;
		UniqueId id = 0;
	} uniqueIdProvider;

};

UniqueObjectManager MQTTAsync::Priv::UniqueIdProvider::uom;

MQTTAsync::MQTTAsync(const std::string & name) : priv(new Priv(this, name))
{ }

MQTTAsync::MQTTAsync(const Logging::LoggerPtr & logger) : priv(new Priv(this, logger))
{ }

MQTTAsync::~MQTTAsync()
{
	deinit();
	delete priv;
}


//static
void MQTTAsync::globalInit()
{
//	MQTTAsync_init_options opts = MQTTAsync_init_options_initializer;
//	MQTTAsync_global_init(&opts);
}

bool MQTTAsync::init(const MQTTConnectionOptions & conn_opts, ErrorCollector & ec)
{
	SAS_LOG_NDC();

	UniqueId uid;
	if (!(uid = priv->uniqueIdProvider.get(ec)))
		return false;

	priv->client_id = conn_opts.clientId + "_" + std::to_string(uid);

	SAS_LOG_VAR(priv->logger, conn_opts.serverUri);
	SAS_LOG_VAR(priv->logger, priv->client_id);

	int rc;
	SAS_LOG_TRACE(priv->logger, "MQTTAsync_create");
	if ((rc = MQTTAsync_create(&priv->mqtt_handle, conn_opts.serverUri.c_str(), priv->client_id.c_str(), MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTASYNC_SUCCESS)
	{
		auto err = ec.add(-1, "could not initialize MQTT ("+std::to_string(rc)+")");
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}

	SAS_LOG_TRACE(priv->logger, "MQTTAsync_setCallbacks, MQTTAsync_setConnected");
	if ((rc = MQTTAsync_setCallbacks(priv->mqtt_handle, priv, Priv::_connectionLost, Priv::_messageArrived, Priv::_deliveryComplete)) != MQTTASYNC_SUCCESS ||
	    (rc = MQTTAsync_setConnected(priv->mqtt_handle, priv, Priv::_connected)) != MQTTASYNC_SUCCESS)
	{
		auto err = ec.add(-1, "unexpected error could not set MQTT callbacks ("+std::to_string(rc)+")");
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}

	priv->options = conn_opts;

	return true;
}

void MQTTAsync::deinit()
{
	SAS_LOG_NDC();
	if (priv->mqtt_handle)
	{
		NullEC ec;
		disconnect(ec);
		SAS_LOG_TRACE(priv->logger, "MQTTAsync_destroy");
		MQTTAsync_destroy(&priv->mqtt_handle);
		priv->mqtt_handle = nullptr;
	}
}


bool MQTTAsync::connect(ErrorCollector & ec)
{
	SAS_LOG_NDC();
	return priv->connect(ec);
}

bool MQTTAsync::disconnect(ErrorCollector & ec)
{
	SAS_LOG_NDC();

	if (!shutdown(ec))
		return false;

	std::unique_lock<std::mutex> __locker(priv->mut);

	int rc;
	SAS_LOG_TRACE(priv->logger, "MQTTAsync_disconnect");
	MQTTAsync_disconnectOptions ops = MQTTAsync_disconnectOptions_initializer;
	priv->ec = &ec;
	ops.context = priv;
	ops.onSuccess = Priv::_onDisconnected;
	ops.onFailure = Priv::_onDisconnectFailed;
	if ((rc = MQTTAsync_disconnect(priv->mqtt_handle, &ops)) != MQTTASYNC_SUCCESS)
	{
		auto err = ec.add(-1, "could not disconnect from MQTT server (" + std::to_string(rc) + ")");
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}
	priv->disconn_not.wait();

	return true;
}

bool MQTTAsync::subscribe(const std::vector<std::string> & topics, int qos, ErrorCollector & ec)
{
	SAS_LOG_NDC();

	SAS_LOG_TRACE(priv->logger, "MQTTAsync_isConnected");
	if(!MQTTAsync_isConnected(priv->mqtt_handle) && !connect(ec))
		return false;

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
	SAS_LOG_TRACE(priv->logger, "MQTTAsync_unsubscribeMany");
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
	SAS_LOG_TRACE(priv->logger, "MQTTAsync_send");
	if((rc = MQTTAsync_send(priv->mqtt_handle, topic.c_str(), payload.size()-1, (void*)payload.data(), qos, 0, &resp)) != MQTTASYNC_SUCCESS)
	{
		auto err = ec.add(-1, "could not send MQTT message ("+std::to_string(rc)+")");
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}
	if (priv->options.publish_timeout > 0)
	{
		SAS_LOG_TRACE(priv->logger, "MQTTAsync_waitForCompletion");
		if ((rc = MQTTAsync_waitForCompletion(priv->mqtt_handle, resp.token, priv->options.publish_timeout) != MQTTASYNC_SUCCESS))
		{
			auto err = ec.add(-1, "MQTT message is lost (" + std::to_string(rc) + ")");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}
	}

	return true;
}

bool MQTTAsync::run(ErrorCollector & ec)
{
	//SAS_LOG_NDC();
	std::unique_lock<std::mutex> __locker(priv->mut);
	priv->ec = &ec;
	priv->runner_not.wait();
	priv->ec = nullptr;
	return true;
}

bool MQTTAsync::shutdown(ErrorCollector & ec)
{
	//SAS_LOG_NDC();
	priv->runner_not.notify();
	return true;
}

const MQTTConnectionOptions & MQTTAsync::connectOptions() const
{
	return priv->options;
}

}
