/*
This file is part of sasODBC.

sasODBC is free software: you can redistribute it and/or modify
it under the terms of the Lesser GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

sasODBC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with sasODBC.  If not, see <http://www.gnu.org/licenses/>
*/

#include "odbctools.h"
#include <sasCore/errorcollector.h>
#include <sstream>

namespace SAS {
	namespace ODBCTools {

		bool getError(SQLHENV env, SQLHDBC conn, SQLHSTMT stmt, SQLRETURN rc, std::string & err, ErrorCollector & ec)
		{
			SQLCHAR out_sql_state[6];
			SQLINTEGER out_native_error;
			SQLCHAR out_msg_text[1024];
			SQLSMALLINT out_text_length;
			SQLRETURN _rc;

			std::stringstream ss;

			if (!SQL_SUCCEEDED(_rc = SQLError(env, conn, stmt, out_sql_state, &out_native_error, out_msg_text, sizeof(out_msg_text), &out_text_length)))
			{
				ss << "could not get ODBC error info" << " (" << _rc << ")";
				ec.add(-1, ss.str());
				return false;
			}

			std::string tmp_msg;
			tmp_msg.append((const char *)out_msg_text, (size_t)out_text_length);
			ss << "[" << out_sql_state << "] " << tmp_msg << " (" << rc << ")";
			err = ss.str();
			return true;
		}

		std::string getError(SQLHENV env, SQLHDBC conn, SQLHSTMT stmt, SQLRETURN rc, ErrorCollector & ec)
		{
			std::string ret;
			if (!getError(env, conn, stmt, rc, ret, ec))
				return "(error: " + std::to_string(rc) + ")";
			return ret;
		}

		std::string getError(SQLHENV env, SQLHDBC conn, SQLHSTMT stmt, SQLRETURN rc)
		{
			NullEC ec;
			return getError(env, conn, stmt, rc, ec);
		}

}}
