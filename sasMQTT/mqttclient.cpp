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

#include "include/sasMQTT/mqttclient.h"

#include <sasCore/logging.h>
#include <sasCore/errorcollector.h>
#include <sasCore/thread.h>
#include <sasCore/notifier.h>

#include "include/sasMQTT/mqttconnectionoptions.h"

#include "include/sasMQTT/mqttasync.h"

#include <memory>
#include <mutex>
#include <string.h>

#include <iostream>

namespace SAS {

	struct MQTTClient_priv
	{
		MQTTClient_priv(const std::string & name_) :
            logger(Logging::getLogger("SAS.MQTTClient[" + name_ + "]")),
			async(this, name_)
		{ }

		MQTTClient_priv(const Logging::LoggerPtr & logger_) :
			logger(logger_),
			async(this, logger_)
		{ }

		Logging::LoggerPtr logger;

		MQTTConnectionOptions options;

		class Async : public MQTTAsync
		{
		private:
			struct Ticket
			{
				std::string topic;
				std::vector<char> payload;
				int qos = 0;

				Notifier notifier;
			};

			std::mutex ticket_mut;
			Ticket * ticket = nullptr;

			MQTTClient_priv * priv;
		public:
			Async(MQTTClient_priv * priv_, const std::string & name) : MQTTAsync(name), priv(priv_)
			{ }

			Async(MQTTClient_priv * priv_, const Logging::LoggerPtr & logger) : MQTTAsync(logger), priv(priv_)
			{ }

			bool exchange(const std::string & in_topic, const std::vector<char> & in_payload, long in_qos, const std::vector<std::string> & subscribe, long ss_qos, std::string & out_topic, std::vector<char> & out_payload, int & out_qos, long count, ErrorCollector & ec)
			{
				Ticket t;
//				t.not.wait();

				{
					std::unique_lock<std::mutex> __ticket_locker(ticket_mut);
					if (!this->subscribe(subscribe, ss_qos, ec))
						return false;
					ticket = &t;
				}

				if (in_topic.length() && !send(in_topic, in_payload, in_qos, ec))
				{
					this->unsubscribe(ec);
					return false;
				}

				long i = 0;
				while (count < 0 || i < count)
				{
					if (count >= 0)
						++i;

                    if (t.notifier.wait(this->connectOptions().receiveTimeout()))
					{
						std::unique_lock<std::mutex> __ticket_locker(ticket_mut);
						this->unsubscribe(ec);
						ticket = nullptr;

//						t.not.notify();
						out_topic = t.topic;
						out_payload = t.payload;
						out_qos = t.qos;
						return true;
					}
				}
				this->unsubscribe(ec);

				auto err = ec.add(-1, "could not get MQTT message: timeout reached");
				SAS_LOG_ERROR(priv->logger, err);

				std::unique_lock<std::mutex> __ticket_locker(ticket_mut);
				ticket = nullptr;

				return false;
			}

		protected:
			virtual bool messageArrived(const std::string & topic, const std::vector<char> & payload, int qos) override
			{
				if (!ticket_mut.try_lock())
					return false;
				if (ticket)
				{
					ticket->topic = topic;
					ticket->payload = payload;
					ticket->qos = qos;
					ticket->notifier.notify();
					ticket_mut.unlock();
					return true;
				}
				ticket_mut.unlock();
				return false;
			}
		} async;
		
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
		return priv->async.init(options, ec);
	}

	void MQTTClient::deinit()
	{
		SAS_LOG_NDC();
		priv->async.deinit();
	}

	bool MQTTClient::publish(const std::string & topic, const std::vector<char> & payload, long qos, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		return priv->async.send(topic, payload, qos, ec);
	}

	bool MQTTClient::receive(const std::vector<std::string> & subscribe, long ss_qos, std::string & topic, std::vector<char> & payload, long count, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		int tmp_qos;
		return priv->async.exchange(std::string(), std::vector<char>(), 0, subscribe, ss_qos, topic, payload, tmp_qos, count, ec);
	}

    bool MQTTClient::receive(const std::vector<std::string> & subscribe, long qos, std::string & topic, std::vector<char> & payload, ErrorCollector & ec)
	{
        return receive(subscribe, qos, topic, payload, -1, ec);
	}

	bool MQTTClient::exchange(const std::string & in_topic, const std::vector<char> & in_payload, long in_qos,
			const std::vector<std::string> & subscribe, long ss_qos, std::string & out_topic, std::vector<char> & out_payload, long count,
			ErrorCollector & ec)
	{
		SAS_LOG_NDC();

		int tmp_qos;
		return priv->async.exchange(in_topic, in_payload, in_qos, subscribe, ss_qos, out_topic, out_payload, tmp_qos, count, ec);
	}

	bool MQTTClient::connect(ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		return priv->async.connect(ec);
	}

	bool MQTTClient::disconnect(long timeout, ErrorCollector & ec)
	{
        (void)timeout;
		SAS_LOG_NDC();
		return priv->async.disconnect(ec);
	}

}
