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
#include <sasCore/tools.h>

#include <memory>

namespace SAS {

bool SQLConnector::completeStatement(std::string & sql, ErrorCollector & ec) const
{
	SAS_LOG_NDC();

	struct Local
	{
		const SQLConnector* that;

		bool replace(std::string & sql, ErrorCollector & ec)
		{
			short br_cnt = 0;
			bool qm = false;
			bool ap = false;
			bool esc = false;
			bool cmd = false;

			std::string ret;

			std::string command;
			std::vector<std::string> args;
			std::string* _arg = nullptr;
			for (auto c : sql)
			{
				if (esc)
				{
					esc = false;
					goto add;
				}
				else if (c == '$' && !cmd && !qm && !ap && br_cnt == 0)
					cmd = true;
				else switch (c)
				{
				case '\\':
					if (br_cnt == 0)
					{
						esc = true;
						break;
					}
					goto add;
				case '"':
					if (!ap)
						qm = !qm;
					goto add;
				case '\'':
					if (!qm)
						ap = !ap;
					goto add;
				case '{':
					switch (++br_cnt)
					{
					case 1:
						if (cmd)
							_arg = &command;
						break;
					case 2:
						args.push_back(std::string());
						_arg = &args.back();
						break;
					default:
						goto add;
					}
					break;
				case '}':
					switch (--br_cnt)
					{
					case 0:
						for (auto& a : args)
							if (!replace(a, ec))
								return false;


						if (!that->appendCompletionValue(command, args, ret, ec))
							return false;

						cmd = false;
						args.clear();
						command.clear();
						_arg = nullptr;

						break;
					case 1:
						_arg = nullptr; break;
					case -1:
						goto end;
					default:
						goto add;
					}
					break;
				default:
				add:
					if (_arg)
						*_arg += c;
					else if (!cmd)
						ret += c;
				}
			}

		end:
			sql = ret;
			if (br_cnt != 0 || ap || qm || esc)
			{
				ec.add(-1, "synax error in input statement");
				return false;
			}

			return true;
		}
	} local { this };

	return local.replace(sql, ec);
}
 
bool SQLConnector::getSysDate(SAS::SQLDateTime & ret, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	std::unique_ptr<SQLStatement> stmt(createStatement(ec));
	if (!stmt)
		return false;
	return stmt->getSysDate(ret, ec);
}


struct SQLTransactionProtector::Priv
{
	Priv(SQLConnector * conn_, bool auto_commit_) :
		conn(conn_),
		auto_commit(auto_commit_),
		ended(false)
	{ }

	SQLConnector * conn;
	bool auto_commit;
	bool ended;

	struct SimpleEC : public ErrorCollector
	{
        virtual inline ~SimpleEC() override = default;
        virtual inline void append(long errorCode, const std::string & errorText) override
        {
            (void)errorCode;
            (void)errorText;
        }
	} s_ec;
};

SQLTransactionProtector::SQLTransactionProtector(SQLConnector * conn, bool auto_commit) :
		priv(new Priv(conn, auto_commit))
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
