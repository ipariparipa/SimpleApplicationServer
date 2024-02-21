/*
    This file is part of sasOracle.

    sasOracle is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasOracle is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasOracle.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef sasOracle__oraconnector_h
#define sasOracle__oraconnector_h

#include "config.h"
#include <sasSQL/sqlconnector.h>
#include <sasCore/logging.h>
#include SAS_ORACLE__DPI_H

#include <mutex>

namespace SAS {

class Application;

struct Oracle_Settings
{
	size_t max_buffer_size = 0;
	size_t max_connections = 0;
};

class OraConnector : public SQLConnector
{
	SAS_COPY_PROTECTOR(OraConnector)

	struct Priv;
	Priv * priv;
public:
	OraConnector(dpiContext * ctx, const std::string & name, Application * app);
	virtual ~OraConnector() override;

	inline const char * getServerType() const final override
	{ return "oracle"; }

	bool getServerInfo(std::string & generation, std::string & version, ErrorCollector & ec) final override;

	bool hasFeature(Feature f, std::string & explanation) final override;

	bool init(const std::string & configPath, ErrorCollector & ec);

	bool connect(ErrorCollector & ec) final override;

	SQLStatement * createStatement(ErrorCollector & ec) final override;

	std::string name() const final override;

	bool exec(const std::string & statement, SQLResult *& res, ErrorCollector & ec) final override;
	bool exec(const std::string & statement, ErrorCollector & ec) final override;

	void detach() final override;

	Logging::LoggerPtr logger() const;

	const Oracle_Settings & settings() const;

	std::mutex & mutex();

	bool activate(ErrorCollector & ec) final override;

	void lock() final override;
	void unlock() final override;

	bool startTransaction(ErrorCollector & ec) final override;
	bool commit(ErrorCollector & ec) final override;
	bool rollback(ErrorCollector & ec) final override;

	dpiConn * conn();
	dpiConn * conn(ErrorCollector & ec);
	dpiContext * ctx() const;

	std::string getErrorText();

protected:
	bool appendCompletionValue(const std::string& command, const std::vector<std::string>& args, std::string& ret, ErrorCollector& ec) const final override;
};

}

#endif // sasOracle__oraconnector_h
