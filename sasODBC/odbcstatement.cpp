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

#include "odbcstatement.h"
#include "odbcconnector.h"
#include <sasCore/logging.h>
#include "include_odbc.h"

#include <sasCore/errorcollector.h>
#include <sasSQL/errorcodes.h>
#include <sasSQL/sqldatetime.h>

#include <assert.h>
#include <string.h>
#include <limits.h>

#include <regex>

namespace SAS {

	struct ODBCStatement::Priv
	{
		Priv(ODBCConnector * conn_) : 
			conn(conn_),
			logger(conn_->logger())
		{ }

		ODBCConnector * conn;
        SQLHSTMT stmt = nullptr;
		Logging::LoggerPtr logger;
		
		struct Bind
		{
			std::vector<unsigned char> _data;
			SQLLEN _ind = 0;

			SQLLEN * ind()
			{
				return _ind == SQL_NULL_DATA ? &_ind : NULL;
			}

			void* data()
			{
				return static_cast<void*>(_data.data());
			}

			template<typename T>
			void alloc(size_t size = 1)
			{
				_ind = 0;
				_data.resize(size * sizeof(T));
			}

			template<typename T>
			void setData(const T* src, size_t size = 1)
			{
				alloc<T>(size);
				memcpy(_data.data(), src, size * sizeof(T));
			}

			template<typename T>
			void setNull()
			{
				_data.resize(sizeof(T));
				_ind = SQL_NULL_DATA;
			}
		};
		std::vector<Bind> bind_buffer;

		typedef std::tuple<std::string /*0 - field name*/, SQLDataType /*1*/, size_t /*2 - size*/> Field;
		std::vector<Field> res_fields;
		SQLLEN row_num = 0;

		template<typename T>
		bool bindParam(size_t idx, const T& val, bool isNull, ErrorCollector& ec)
		{
			static_assert("not implemented");
		}

		template<>
		bool bindParam<SQLBIGINT>(size_t idx, const SQLBIGINT & val, bool isNull, ErrorCollector & ec)
		{
			switch (conn->settings().int64BindRule)
			{
			case ODBC_Settings::Int64BindRule::NotSupported:
			{
				auto err = ec.add(SAS_SQL__ERROR__CANNOT_BIND_PARAMETERS, "binding of int64 value is not supported");
				SAS_LOG_ERROR(logger, err);
				return false;
			}
			case ODBC_Settings::Int64BindRule::Normal:
			{
				SQLRETURN rc;

				assert(idx < bind_buffer.size());
				assert(stmt);

				auto& buff = bind_buffer[idx];
				if (isNull)
					buff.setNull<SQLBIGINT>();
				else
					buff.setData<SQLBIGINT>(&val);

				SAS_LOG_TRACE(logger, "SQLBindParameter");
				if (!(SQL_SUCCEEDED(rc = SQLBindParameter(stmt,
				                                          static_cast<SQLUSMALLINT>(idx + 1),
				                                          SQL_PARAM_INPUT,
				                                          SQL_C_SBIGINT,
				                                          SQL_BIGINT,
				                                          0,
				                                          0,
				                                          buff.data(),
				                                          0,
				                                          buff.ind()))))
				{
					auto err = ec.add(SAS_SQL__ERROR__CANNOT_BIND_PARAMETERS, conn->getErrorText(stmt, rc, ec));
					SAS_LOG_ERROR(logger, err);
					return false;
				}

				return true;
			}
			case ODBC_Settings::Int64BindRule::AsInt32_or_AsString:
				if (llabs(val) > LONG_MAX)
					return bindParam(idx, std::to_string(val), isNull, ec);

				return bindParam<SQLINTEGER>(idx, static_cast<SQLINTEGER>(val), isNull, ec);
			case ODBC_Settings::Int64BindRule::AsInt32:
				if (llabs(val) > LONG_MAX)
				{
					auto err = ec.add(SAS_SQL__ERROR__CANNOT_BIND_PARAMETERS, "int64 value cannot be bound");
					SAS_LOG_ERROR(logger, err);
					return false;
				}

				return bindParam<SQLINTEGER>(idx, static_cast<SQLINTEGER>(val), isNull, ec);
			case ODBC_Settings::Int64BindRule::AsString:
				return bindParam(idx, std::to_string(val), isNull, ec);
			}

			auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "unexpected error");
			SAS_LOG_ERROR(logger, err);

			return false;
		}

		template<>
		bool bindParam<SQLINTEGER>(size_t idx, const SQLINTEGER& val, bool isNull, ErrorCollector& ec)
		{
			assert(idx < bind_buffer.size());
			assert(stmt);

			auto& buff = bind_buffer[idx];
			if (isNull)
				buff.setNull<SQLINTEGER>();
			else
				buff.setData<SQLINTEGER>(&val);

			SQLRETURN rc;
			SAS_LOG_TRACE(logger, "SQLBindParameter");
			if (!(SQL_SUCCEEDED(rc = SQLBindParameter(stmt,
			                                          static_cast<SQLUSMALLINT>(idx + 1),
			                                          SQL_PARAM_INPUT,
			                                          SQL_C_SLONG,
			                                          SQL_INTEGER,
			                                          0,
			                                          0,
			                                          buff.data(),
			                                          0,
			                                          buff.ind()))))
			{
				auto err = ec.add(SAS_SQL__ERROR__CANNOT_BIND_PARAMETERS, conn->getErrorText(stmt, rc, ec));
				SAS_LOG_ERROR(logger, err);
				return false;
			}

			return true;
		}

		template<>
		bool bindParam<SQLDOUBLE>(size_t idx, const SQLDOUBLE& val, bool isNull, ErrorCollector & ec)
		{
			assert(idx < bind_buffer.size());
			assert(stmt);

			auto & buff = bind_buffer[idx];
			if (isNull)
				buff.setNull<SQLDOUBLE>();
			else
				buff.setData<SQLDOUBLE>(&val);

			SQLRETURN rc;
			SAS_LOG_TRACE(logger, "SQLBindParameter");
			if (!(SQL_SUCCEEDED(rc = SQLBindParameter(stmt,
			                                          static_cast<SQLUSMALLINT>(idx + 1),
			                                          SQL_PARAM_INPUT,
			                                          SQL_C_DOUBLE,
			                                          SQL_DOUBLE,
			                                          0,
			                                          0,
			                                          buff.data(),
			                                          0,
			                                          buff.ind()))))
			{
				auto err = ec.add(SAS_SQL__ERROR__CANNOT_BIND_PARAMETERS, conn->getErrorText(stmt, rc, ec));
				SAS_LOG_ERROR(logger, err);
				return false;
			}

			return true;
		}

		template<>
		bool bindParam<std::string>(size_t idx, const std::string & val, bool isNull, ErrorCollector & ec)
		{
			SAS_LOG_NDC();

			assert(idx < bind_buffer.size());
			assert(stmt);

			auto& buff = bind_buffer[idx];
			if (isNull)
				buff.setNull<char>();
			else
				buff.setData<char>(val.c_str(), val.length());

			SQLRETURN rc;
			SAS_LOG_TRACE(logger, "SQLBindParameter");
			if (!(SQL_SUCCEEDED(rc = SQLBindParameter(stmt,
			                                          static_cast<SQLUSMALLINT>(idx + 1),
				                                      SQL_PARAM_INPUT,
			                                          SQL_C_CHAR,
			                                          val.length() > 254 ? SQL_LONGVARCHAR : SQL_VARCHAR,
			                                          static_cast<SQLUINTEGER>(val.length()),
			                                          0,
			                                          buff.data(),
			                                          static_cast<SQLUINTEGER>(val.length()),
			                                          buff.ind()))))
			{
				auto err = ec.add(SAS_SQL__ERROR__CANNOT_BIND_PARAMETERS, conn->getErrorText(stmt, rc, ec));
				SAS_LOG_ERROR(logger, err);
				return false;
			}

			return true;
		}

		template<>
		bool bindParam<std::vector<char>>(size_t idx, const std::vector<char> & val, bool isNull, ErrorCollector & ec)
		{
			return bindParam(idx, val.size(), (const unsigned char*)val.data(), isNull, ec);
		}

		template<>
		bool bindParam<std::vector<unsigned char>>(size_t idx, const std::vector<unsigned char> & val, bool isNull, ErrorCollector & ec)
		{
			return bindParam(idx, val.size(), val.data(), isNull, ec);
		}

		template<typename T>
		bool bindParam(size_t idx, size_t size, const T* buffer, bool isNull, ErrorCollector& ec)
		{
			static_assert("not implemented");
		}

		template<>
		bool bindParam<unsigned char>(size_t idx, size_t size, const unsigned char * buffer, bool isNull, ErrorCollector & ec)
		{
			SQLRETURN rc;

			assert(idx < bind_buffer.size());
			assert(stmt);

			auto & buff = bind_buffer[idx];
			if (isNull)
				buff.setNull<unsigned char>();
			else
				buff.setData<unsigned char>(buffer, size);

			SAS_LOG_TRACE(logger, "SQLBindParameter");
			if (!(SQL_SUCCEEDED(rc = SQLBindParameter(stmt,
			                                          static_cast<SQLUSMALLINT>(idx + 1),
			                                          SQL_PARAM_INPUT,
			                                          SQL_C_BINARY,
			                                          SQL_VARBINARY,
			                                          static_cast<SQLUINTEGER>(size + 1),
			                                          0,
			                                          buff.data(),
			                                          static_cast<SQLUINTEGER>(size + 1),
			                                          buff.ind()))))
			{
				auto err = ec.add(SAS_SQL__ERROR__CANNOT_BIND_PARAMETERS, conn->getErrorText(stmt, rc, ec));
				SAS_LOG_ERROR(logger, err);
				return false;
			}

			return true;
		}

		bool bindParam(size_t idx, const tm & val, unsigned int nanosec, bool isNull, ErrorCollector & ec)
		{
			SQLRETURN rc;

			assert(idx < bind_buffer.size());

			int precision = conn->settings().info.dtprec - 20; // (20 includes a separating period)

			auto& buff = bind_buffer[idx];
			if (isNull)
				buff.setNull<TIMESTAMP_STRUCT>();
			else
			{
				buff.alloc<TIMESTAMP_STRUCT>();
				TIMESTAMP_STRUCT * dt = static_cast<TIMESTAMP_STRUCT *>(buff.data());

                dt->year = static_cast<SQLSMALLINT>(val.tm_year + 1900);
                dt->month = static_cast<SQLUSMALLINT>(val.tm_mon + 1);
                dt->day = static_cast<SQLUSMALLINT>(val.tm_mday);
                dt->hour = static_cast<SQLUSMALLINT>(val.tm_hour);
                dt->minute = static_cast<SQLUSMALLINT>(val.tm_min);
                dt->second = static_cast<SQLUSMALLINT>(val.tm_sec);
				if (precision <= 0)
					dt->fraction = 0;
				else
				{
                    dt->fraction = static_cast<SQLUSMALLINT>(nanosec);

					//// (How many leading digits do we want to keep?  With SQL Server 2005, this should be 3: 123000000)
					//int keep = (int)qPow(10.0, 9 - qMin(9, precision));
					//dt->fraction = (dt->fraction / keep) * keep;
				}
			}

			SAS_LOG_TRACE(logger, "SQLBindParameter");
			if (!(SQL_SUCCEEDED(rc = SQLBindParameter(stmt,
			                                          static_cast<SQLUSMALLINT>(idx + 1),
			                                          SQL_PARAM_INPUT,
			                                          SQL_C_TIMESTAMP,
			                                          SQL_TIMESTAMP,
			                                          static_cast<SQLULEN>(conn->settings().info.dtprec),
			                                          static_cast<SQLSMALLINT>(precision),
			                                          buff.data(),
			                                          0,
			                                          buff.ind()))))
			{
				auto err = ec.add(SAS_SQL__ERROR__CANNOT_BIND_PARAMETERS, conn->getErrorText(stmt, rc, ec));
				SAS_LOG_ERROR(logger, err);
				return false;
			}

			return true;
		}

		bool getFieldNum(size_t & ret, ErrorCollector & ec)
		{
			SAS_LOG_NDC();

			assert(conn);
			SAS_LOG_ASSERT(logger, stmt, "statement cannot be NULL");

			SQLRETURN rc;
			SQLSMALLINT res_col_num;

			SAS_LOG_TRACE(logger, "SQLNumResultCols");
			if (!SQL_SUCCEEDED(rc = SQLNumResultCols(stmt, &res_col_num)))
			{
				auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "could not get number of fields: " + conn->getErrorText(stmt, rc, ec));
				SAS_LOG_ERROR(logger, err);
				return false;
			}

            ret = static_cast<size_t>(res_col_num);

			return true;
		}

		bool getFields(std::vector<Field> & ret, ErrorCollector & ec)
		{
			SAS_LOG_NDC();

			assert(conn);
			SAS_LOG_ASSERT(logger, stmt, "statement cannot be NULL");

			SQLRETURN rc;

			size_t res_col_num;
			if (!getFieldNum(res_col_num, ec))
				return false;

			ret.resize(res_col_num);

			SQLSMALLINT type;
			SQLUINTEGER size;
			SQLCHAR col_name[1024];
			SQLSMALLINT col_name_length;
			SQLSMALLINT dec_digits;
			SQLSMALLINT nullable;
			bool has_error = false;
			for (size_t i = 0; i < res_col_num; ++i)
			{
				auto & r = ret[i];

				SAS_LOG_TRACE(logger, "SQLDescribeCol");
                if (!SQL_SUCCEEDED(rc = SQLDescribeCol(stmt, static_cast<SQLUSMALLINT>(i + 1), col_name, sizeof(col_name), &col_name_length, &type, &size, &dec_digits, &nullable)))
				{
					auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "could not get number of parameter fo binding: " + conn->getErrorText(stmt, rc, ec));
					SAS_LOG_ERROR(logger, err);
					has_error = true;
				}
				else
				{
					switch (type)
					{
					case SQL_CHAR:
					case SQL_VARCHAR:
					case SQL_LONGVARCHAR:
					case SQL_WCHAR:
					case SQL_WVARCHAR:
					case SQL_WLONGVARCHAR:
						std::get<1>(r) = SQLDataType::String;
						break;
					case SQL_SMALLINT:
					case SQL_INTEGER:
					case SQL_TINYINT:
					case SQL_BIGINT:
						std::get<1>(r) = SQLDataType::Number;
						break;
					case SQL_DECIMAL:
					case SQL_NUMERIC:
					case SQL_REAL:
					case SQL_FLOAT:
					case SQL_DOUBLE:
						std::get<1>(r) = SQLDataType::Real;
						break;
					case SQL_BIT:
						std::get<1>(r) = SQLDataType::Number;
						break;
					case SQL_BINARY:
					case SQL_VARBINARY:
					case SQL_LONGVARBINARY:
						std::get<1>(r) = SQLDataType::Blob;
						break;
					case SQL_TYPE_DATE:
					case SQL_TYPE_TIME:
					case SQL_TYPE_TIMESTAMP:
					case SQL_DATE:
					case SQL_TIME:
					case SQL_TIMESTAMP:
						std::get<1>(r) = SQLDataType::DateTime;
						break;
						//case SQL_TYPE_UTCDATETIME:
						//case SQL_TYPE_UTCTIME:
						//	break;
					case SQL_INTERVAL_MONTH:
					case SQL_INTERVAL_YEAR:
					case SQL_INTERVAL_YEAR_TO_MONTH:
					case SQL_INTERVAL_DAY:
					case SQL_INTERVAL_HOUR:
					case SQL_INTERVAL_MINUTE:
					case SQL_INTERVAL_SECOND:
					case SQL_INTERVAL_DAY_TO_HOUR:
					case SQL_INTERVAL_DAY_TO_MINUTE:
					case SQL_INTERVAL_DAY_TO_SECOND:
					case SQL_INTERVAL_HOUR_TO_MINUTE:
					case SQL_INTERVAL_HOUR_TO_SECOND:
					case SQL_INTERVAL_MINUTE_TO_SECOND:
						std::get<1>(r) = SQLDataType::Number;
						break;
					case SQL_GUID:
					default:
						auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "unsuipported ODBC data type: '" + std::to_string(type) + "'");
						SAS_LOG_ERROR(logger, err);
						has_error = true;
					}

					std::get<0>(r) = (const char *)col_name;
                    std::get<2>(r) = static_cast<size_t>(size);
				}
			}

			return !has_error;
		}

		bool reset(ErrorCollector& ec)
		{
			if (stmt)
			{
				SQLRETURN rc;
				if (!SQL_SUCCEEDED(rc = SQLFreeHandle(SQL_HANDLE_STMT, stmt)))
				{
					auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "unable to free the statement: " + conn->getErrorText(SQL_NULL_HANDLE, rc, ec));
					SAS_LOG_ERROR(logger, err);
					return false;
				}

				stmt = nullptr;
			}

			return true;
		}
	};

	ODBCStatement::ODBCStatement(ODBCConnector * conn) : SQLStatement(), priv(new Priv(conn))
	{ }

	ODBCStatement::~ODBCStatement()
	{
		priv->reset(NullEC());
		delete priv;
	}

	bool ODBCStatement::init(ErrorCollector & ec)
	{
		SAS_LOG_NDC();

		return true;
	}

	bool ODBCStatement::prepare(const std::string & statement, ErrorCollector & ec)
	{
		SQLHDBC conn;
		if (!(conn = priv->conn->conn(ec)))
			return false;

		priv->res_fields.clear();
		priv->bind_buffer.clear();
		priv->row_num = 0;

		if (!priv->reset(ec))
			return false;

		SQLRETURN rc;

		SAS_LOG_TRACE(priv->logger, "SQLAllocHandle");
		if (!SQL_SUCCEEDED(rc = SQLAllocHandle(SQL_HANDLE_STMT, conn, &priv->stmt)))
		{
			auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "could not allocate handle for statement: " + priv->conn->getErrorText(SQL_NULL_HANDLE, rc, ec));
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}

		SAS_LOG_ASSERT(priv->logger, priv->stmt, "statement cannot be NULL");

		SAS_LOG_TRACE(priv->logger, "SQLPrepare");
        if (!SQL_SUCCEEDED(rc = SQLPrepare(priv->stmt, (SQLCHAR*)statement.c_str(), static_cast<SQLINTEGER>(statement.length()))))
		{
			auto err = ec.add(SAS_SQL__ERROR__CANNOT_PREPARE_STATEMENT, priv->conn->getErrorText(priv->stmt, rc, ec));
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}

		SQLSMALLINT numParams;
		SAS_LOG_TRACE(priv->logger, "SQLNumParams");
		if (!SQL_SUCCEEDED(rc = SQLNumParams(priv->stmt, &numParams)))
		{
			auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "could not get number of parameter fo binding: " + priv->conn->getErrorText(priv->stmt, rc, ec));
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}

        priv->bind_buffer.resize(static_cast<size_t>(numParams));

		return true;
	}

	unsigned long ODBCStatement::paramNum()
	{
		return priv->bind_buffer.size();
	}

	bool ODBCStatement::bindParam(const std::vector<SQLVariant> & params, ErrorCollector & ec)
	{
		SAS_LOG_NDC();

		assert(priv->conn);
		SAS_LOG_ASSERT(priv->logger, priv->stmt, "statement cannot be NULL");

		auto params_size = params.size();
		auto stmt_params_size = priv->bind_buffer.size();

		if (stmt_params_size > params_size)
		{
			SAS_LOG_VAR(priv->conn->logger(), stmt_params_size);
			SAS_LOG_VAR(priv->conn->logger(), params_size);
			auto err = ec.add(SAS_SQL__ERROR__CANNOT_BIND_PARAMETERS, "insufficient number of parameters");
			SAS_LOG_ERROR(priv->conn->logger(), err);
			return false;
		}
		else if (stmt_params_size != params_size)
		{
			SAS_LOG_VAR(priv->conn->logger(), stmt_params_size);
			SAS_LOG_VAR(priv->conn->logger(), params_size);
			SAS_LOG_WARN(priv->conn->logger(), "incorrect number of parameters to be bound");
		}

		bool has_error = false;
		size_t idx(0);
		for (auto & p : params)
		{
			switch (p.type())
			{
			case SQLDataType::None:
				break;
			case SQLDataType::String:
				if (!priv->bindParam(idx, p.asString(), p.isNull(), ec))
					has_error = true;
				break;
			case SQLDataType::Number:
				if (!priv->bindParam<SQLBIGINT>(idx, p.asNumber(), p.isNull(), ec))
					has_error = true;
				break;
			case SQLDataType::Real:
				if (!priv->bindParam<SQLDOUBLE>(idx, p.asReal(), p.isNull(), ec))
					has_error = true;
				break;
			case SQLDataType::DateTime:
				{
					auto & _dt = p.asDateTime();
                    if (!priv->bindParam(idx, _dt.to_tm(), _dt.nanoseconds(), p.isNull(), ec))
						has_error = true;
					break;
				}
			case SQLDataType::Blob:
				{
                    size_t s = 0;
					if (!priv->bindParam(idx, s, p.asBlob(s), p.isNull(), ec))
						has_error = true;
					break;
				}
			}
			++idx;
		}
		if (has_error)
			return false;

		return true;
	}

	bool ODBCStatement::bindParam(const std::vector<std::pair<std::string /*name*/, SQLVariant>> & params, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		assert(priv->conn);

        (void)params;

		auto err = ec.add(SAS_SQL__ERROR__NOT_SUPPORTED, "functionality is not supported by ODBC library");
		SAS_LOG_ERROR(priv->logger, err);

		return false;
	}

	bool ODBCStatement::execDML(ErrorCollector & ec)
	{
		SAS_LOG_NDC();

		priv->res_fields.clear();
		priv->row_num = 0;

		assert(priv->conn);
		SAS_LOG_ASSERT(priv->logger, priv->stmt, "statement cannot be NULL");

		SQLRETURN rc;
		SAS_LOG_TRACE(priv->logger, "SQLExecute");
		if (!SQL_SUCCEEDED(rc = SQLExecute(priv->stmt)))
		{
			auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "could not execute statement: " + priv->conn->getErrorText(priv->stmt, rc, ec));
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}

		SAS_LOG_TRACE(priv->logger, "SQLRowCount");
		if (!SQL_SUCCEEDED(rc = SQLRowCount(priv->stmt, &priv->row_num)))
		{
			auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "could get count of rows: " + priv->conn->getErrorText(priv->stmt, rc, ec));
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}

		return true;
	}
	
	bool ODBCStatement::exec(ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		if (!execDML(ec) || !priv->getFields(priv->res_fields, ec))
			return false;

		return true;
	}
	
	bool ODBCStatement::getLastGeneratedId(const std::string & schema, const std::string & table, const std::string & field, SQLVariant & ret, ErrorCollector & ec)
	{
		SAS_LOG_NDC();

		assert(priv->conn);

		auto it = priv->conn->settings().statementInjections.find(ODBC_Settings::StatementInjection::GetLastGeneratedId);
		if (it == priv->conn->settings().statementInjections.end())
		{
			auto err = ec.add(SAS_SQL__ERROR__NOT_SUPPORTED, "functionality is not supported by ODBC connector");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}

		auto sql = it->second;
		sql = std::regex_replace(sql, std::regex(R"(\$\(schema\)|\$\{schema\})"), schema);
		sql = std::regex_replace(sql, std::regex(R"(\$\(table\)|\$\{table\})"), table);
		sql = std::regex_replace(sql, std::regex(R"(\$\(field\)|\$\{field\})"), field);

		if (!prepare(sql, ec) || !exec(ec))
			return false;
		std::vector<SAS::SQLVariant> data;
		if (!fetch(data, ec))
			return false;
		SAS_LOG_ASSERT(priv->conn->logger(), data.size() == 1, "invalid data size");
		ret = data.front();
		return true;
	}

	bool ODBCStatement::fieldNum(size_t & ret, ErrorCollector & ec)
	{
		SAS_LOG_NDC();

		if ((ret = priv->res_fields.size()))
			return true;

		return priv->getFieldNum(ret, ec);
	}

	bool ODBCStatement::fields(std::vector<std::tuple<std::string /*db/scheme*/, std::string /*table*/, std::string /*field name*/, SQLDataType>> & ret, ErrorCollector & ec)
	{
		SAS_LOG_NDC();

		std::vector<Priv::Field> tmp_fields;

		std::vector<Priv::Field> * fields;

		if (priv->res_fields.size())
			fields = &(priv->res_fields);
		{
			if (!priv->getFields(tmp_fields, ec))
				return false;
			fields = &tmp_fields;
		}

		ret.resize(fields->size());
		for (size_t i = 0, l = fields->size(); i < l; ++i)
		{			auto & f = (*fields)[i];
			ret[i] = std::make_tuple(std::string(), std::string(), std::get<0>(f), std::get<1>(f));
		}

		return true;
	}

	unsigned long long ODBCStatement::rowNum()
	{
        return static_cast<unsigned long long>(priv->row_num);
	}
	
	bool ODBCStatement::fetch(std::vector<SQLVariant> & ret, ErrorCollector & ec)
	{
		SAS_LOG_NDC();

		assert(priv->conn);
		SAS_LOG_ASSERT(priv->logger, priv->stmt, "statement cannot be NULL");

		if (!priv->res_fields.size())
		{
			auto err = ec.add(SAS_SQL__ERROR__NO_META_INFO, "fields meta info are not set");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}

		SQLRETURN rc;

		SAS_LOG_TRACE(priv->logger, "SQLFetch");
		switch ((rc = SQLFetch(priv->stmt)))
		{
		case SQL_SUCCESS:
		case SQL_SUCCESS_WITH_INFO:
			break;
		case SQL_NO_DATA:
			SAS_LOG_INFO(priv->logger, "end of fetching");
			return false;
		case SQL_STILL_EXECUTING:
		case SQL_ERROR:
		case SQL_INVALID_HANDLE:
		default:
			auto err = ec.add(SAS_SQL__ERROR__NOT_SUPPORTED, "could not fetch record: " + priv->conn->getErrorText(priv->stmt, rc, ec));
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}

		ret.resize(priv->res_fields.size());

		bool has_error = false;
		for (size_t i = 0, l = priv->res_fields.size(); i < l; ++i)
		{
			const auto & f = priv->res_fields[i];
			switch (std::get<1>(priv->res_fields[i]))
			{
			case SQLDataType::None:
				assert("invalid data type");
				has_error = true;
				break;
			case SQLDataType::String:
				{
					std::vector<SQLCHAR> buff(std::get<2>(f));
					SQLINTEGER len;
					SAS_LOG_TRACE(priv->logger, "SQLGetData");
                    switch (rc = SQLGetData(priv->stmt, static_cast<SQLUSMALLINT>(i + 1), SQL_C_CHAR, buff.data(), static_cast<SQLLEN>(buff.size()), &len))
					{
					case SQL_NO_DATA:
						ret[i] = SQLVariant(SQLDataType::String);
						break;
					case SQL_SUCCESS:
						{
							auto s = strlen((const char *)buff.data());
							std::string str;
							str.append((const char*)buff.data(), s < (size_t)len ? s : (size_t)len);
							ret[i] = str;
						}
						break;
					case SQL_STILL_EXECUTING:
					case SQL_ERROR:
					case SQL_INVALID_HANDLE:
					default:
						{
							auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "could not get data: " + priv->conn->getErrorText(priv->stmt, rc, ec));
							SAS_LOG_ERROR(priv->logger, err);
							has_error = true;
						}
					}
				}
				break;
			case SQLDataType::Number:
				{
					SQLBIGINT  buff;
					SQLINTEGER len;
                    switch (rc = SQLGetData(priv->stmt, static_cast<SQLUSMALLINT>(i + 1), SQL_C_SBIGINT, &buff, sizeof(buff), &len))
					{
					case SQL_NO_DATA:
						ret[i] = SQLVariant(SQLDataType::Number);
						break;
					case SQL_SUCCESS:
                        ret[i] = static_cast<long long>(buff);
						break;
					case SQL_STILL_EXECUTING:
					case SQL_ERROR:
					case SQL_INVALID_HANDLE:
					default:
						{
							auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "could not get data: " + priv->conn->getErrorText(priv->stmt, rc, ec));
							SAS_LOG_ERROR(priv->logger, err);
							has_error = true;
						}
					}
				}
				break;
			case SQLDataType::Real:
				{
					SQLDOUBLE buff;
					SQLINTEGER len;
                    switch (rc = SQLGetData(priv->stmt, static_cast<SQLUSMALLINT>(i + 1), SQL_C_DOUBLE, &buff, sizeof(buff), &len))
					{
					case SQL_NO_DATA:
						ret[i] = SQLVariant(SQLDataType::Number);
						break;
					case SQL_SUCCESS:
                        ret[i] = static_cast<double>(buff);
						break;
					case SQL_STILL_EXECUTING:
					case SQL_ERROR:
					case SQL_INVALID_HANDLE:
					default:
						{
							auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "could not get data: " + priv->conn->getErrorText(priv->stmt, rc, ec));
							SAS_LOG_ERROR(priv->logger, err);
							has_error = true;
						}
					}
				}
				break;
			case SQLDataType::DateTime:
				{
					TIMESTAMP_STRUCT buff;
					SQLINTEGER len;
                    switch (rc = SQLGetData(priv->stmt, static_cast<SQLUSMALLINT>(i + 1), SQL_C_TIMESTAMP, &buff, 0, &len))
					{
					case SQL_NO_DATA:
						ret[i] = SQLVariant(SQLDataType::DateTime);
						break;
					case SQL_SUCCESS:
						ret[i] = SQLVariant(SQLDateTime(
                            static_cast<unsigned int>(buff.year), static_cast<unsigned int>(buff.month), static_cast<unsigned int>(buff.day),
                            static_cast<unsigned int>(buff.hour), static_cast<unsigned int>(buff.minute), static_cast<unsigned int>(buff.second),
                            static_cast<unsigned int>(buff.fraction),
                            false, priv->conn->settings().info.dtprec), SQLVariant::DateTimeSubType::DateTime);
						break;
					case SQL_STILL_EXECUTING:
					case SQL_ERROR:
					case SQL_INVALID_HANDLE:
					default:
						{
							auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "could not get data: " + priv->conn->getErrorText(priv->stmt, rc, ec));
							SAS_LOG_ERROR(priv->logger, err);
							has_error = true;
						}
					}
				}
				break;
			case SQLDataType::Blob:
				{
					auto buffer_size = std::get<2>(f);
					if (buffer_size > 65536)
						buffer_size = 65536;

					std::vector<SQLCHAR> buffer(buffer_size);
					SQLINTEGER len;
					size_t read = 0;

                    while ((rc = SQLGetData(priv->stmt, static_cast<SQLUSMALLINT>(i + 1), SQL_C_BINARY, buffer.data() + read, static_cast<SQLINTEGER>(buffer_size), &len)) == SQL_SUCCESS_WITH_INFO || rc == SQL_SUCCESS)
					{
						if (len == SQL_NO_TOTAL || len > SQLLEN(buffer_size))
							read += buffer_size;
						else
                            read += static_cast<size_t>(len);
						if (rc == SQL_SUCCESS)
						{
							buffer.resize(read);
							break;
						}
						buffer.resize(buffer.size() + buffer_size);
					}

					switch (rc)
					{
					case SQL_NO_DATA:
						ret[i] = SQLVariant(SQLDataType::Blob);
						break;
					case SQL_SUCCESS:
						ret[i] = SQLVariant(buffer);
						break;
					case SQL_STILL_EXECUTING:
					case SQL_ERROR:
					case SQL_INVALID_HANDLE:
					default:
						{
							auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "could not get data: " + priv->conn->getErrorText(priv->stmt, rc, ec));
							SAS_LOG_ERROR(priv->logger, err);
							has_error = true;
						}
					}
				}
				break;
			}
		}

		return !has_error;
	}

	bool ODBCStatement::getSysDate(SAS::SQLDateTime & ret, ErrorCollector & ec)
	{
		SAS_LOG_NDC();

		assert(priv->conn);

		auto it = priv->conn->settings().statementInjections.find(ODBC_Settings::StatementInjection::GetSysdate);
		if (it == priv->conn->settings().statementInjections.end())
		{
			auto err = ec.add(SAS_SQL__ERROR__NOT_SUPPORTED, "functionality is not supported by ODBC connector");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}

		std::vector<SAS::SQLVariant> data;
		if (!prepare(it->second, ec) || !exec(ec) || !fetch(data, ec))
			return false;

		SAS_LOG_ASSERT(priv->conn->logger(), data.size() == 1, "invalid length of data");
		ret = data[0].asDateTime();
		return true;
	}

}
