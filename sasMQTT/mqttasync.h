/*
 * mqttasync.h
 *
 *  Created on: 2017.09.19.
 *      Author: apps
 */

#ifndef MQTTASYNC_H_
#define MQTTASYNC_H_

#include "config.h"
#include <sasCore/defines.h>
#include "mqttconnectionoptions.h"
#include <sasCore/logging.h>

#include <memory>
#include <vector>

namespace SAS {

class ErrorCollector;

struct MQTTAsync_priv;

class MQTTAsync
{
	friend struct MQTTAsync_priv;
	SAS_COPY_PROTECTOR(MQTTAsync);
public:
	MQTTAsync(const std::string & name);
	MQTTAsync(const Logging::LoggerPtr & logger);
	virtual ~MQTTAsync();

	static void globalInit();

	bool init(const MQTTConnectionOptions & conn_opts, ErrorCollector & ec);
	void deinit();

	bool connect(ErrorCollector & ec);
	bool disconnect(ErrorCollector & ec);

	bool subscribe(const std::vector<std::string> & topics, int qos, ErrorCollector & ec);
	bool unsubscribe(ErrorCollector & ec);

	bool send(const std::string & topic, const std::vector<char> & payload, int qos, ErrorCollector & ec);

	bool run(ErrorCollector & ec);
	bool shutdown(ErrorCollector & ec);

	const MQTTConnectionOptions & connectOptions() const;
protected:
	virtual bool messageArrived(const std::string & topic, const std::vector<char> & payload, int qos) = 0;

private:
	std::unique_ptr<MQTTAsync_priv> priv;
};

}

#endif /* MQTTASYNC_H_ */
