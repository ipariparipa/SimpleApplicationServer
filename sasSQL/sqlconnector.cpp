/*
    This file is part of sasSQL.

    sasSQL is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasSQL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasSQL.  If not, see <http://www.gnu.org/licenses/>
 */

#include "include/sasSQL/sqlconnector.h"
#include "include/sasSQL/sqlstatement.h"

#include <sasCore/errorcollector.h>
#include <sasCore/logging.h>

namespace SAS {

struct SQLTransactionProtector_priv
{
	SQLTransactionProtector_priv(SQLConnector * conn_, bool auto_commit_) :
		conn(conn_),
		auto_commit(auto_commit_),
		ended(false)
	{ }

	SQLConnector * conn;
	bool auto_commit;
	bool ended;

	struct SimpleEC : public ErrorCollector
	{
		virtual inline void append(long errorCode, const std::string & errorText) override { };
	} s_ec;
};

SQLTransactionProtector::SQLTransactionProtector(SQLConnector * conn, bool auto_commit) :
		priv(new SQLTransactionProtector_priv(conn, auto_commit))
{
	SAS_LOG_NDC();
	priv->conn->lock();
	priv->conn->startTransaction(priv->s_ec);
}

SQLTransactionProtector::~SQLTransactionProtector()
{
	SAS_LOG_NDC();
	if(!priv->ended)
	{
		if(priv->auto_commit)
			priv->conn->commit(priv->s_ec);
		else
			priv->conn->rollback(priv->s_ec);
	}
	priv->conn->unlock();
	delete priv;
}

bool SQLTransactionProtector::commit(ErrorCollector & ec)
{
	SAS_LOG_NDC();
	if(!priv->conn->commit(ec))
		return false;
	priv->ended = true;
	return true;

}

bool SQLTransactionProtector::rollback(ErrorCollector & ec)
{
	SAS_LOG_NDC();
	if(!priv->conn->rollback(ec))
		return false;
	priv->ended = true;
	return true;
}


}
