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

#include "orastatement.h"
#include "oraconnector.h"
#include "oratools.h"

#include <sasCore/errorcollector.h>
#include <sasSQL/sqldatetime.h>
#include <sasSQL/errorcodes.h>

#include <cstring>
#include <memory>
#include <mutex>
#include <utility>
#include SAS_ORACLE__DPI_H
#include <assert.h>

namespace SAS {

struct OraStatement::Priv
{
	Priv(OraConnector * conn_) :
		conn(conn_),
//		conn_mut(conn_->mutex()),
		logger(conn_->logger())
	{ }

	OraConnector * conn;
//	std::mutex & conn_mut;
	std::mutex mut;
	dpiStmt * stmt = nullptr;
	uint32_t query_cnt = 0;
	Logging::LoggerPtr logger;

	std::string getErrorText()
	{
		return conn->getErrorText();
	}

	bool toOraData(const SQLVariant & v, dpiData & d, dpiNativeTypeNum & t, ErrorCollector & ec)
	{
		memset(&d, 0, sizeof(dpiData));

		d.isNull = (int)v.isNull();
		switch (v.type())
		{
		case SQLDataType::None:
			{
				auto err = ec.add(SAS_SQL__ERROR__CANNOT_BIND_PARAMETERS, "invalid variant type: 'None'");
				SAS_LOG_ERROR(logger, err);
				return false;
			}
		case SQLDataType::String:
            d.value.asBytes.encoding = nullptr;
            d.value.asBytes.length = static_cast<uint32_t>(v.asString().length());
			d.value.asBytes.ptr = (char*)v.asString().c_str();
			t = DPI_NATIVE_TYPE_BYTES;
			return true;
		case SQLDataType::Number:
			d.value.asInt64 = v.asNumber();
			t = DPI_NATIVE_TYPE_INT64;
			return true;
		case SQLDataType::Real:
			d.value.asDouble = v.asReal();
			t = DPI_NATIVE_TYPE_DOUBLE;
			return true;
		case SQLDataType::DateTime:
            d.value.asTimestamp.year = static_cast<uint8_t>(v.asDateTime().years());
            d.value.asTimestamp.month = static_cast<uint8_t>(v.asDateTime().months());
            d.value.asTimestamp.day = static_cast<uint8_t>(v.asDateTime().days());
            d.value.asTimestamp.hour = static_cast<uint8_t>(v.asDateTime().hours());
            d.value.asTimestamp.minute = static_cast<uint8_t>(v.asDateTime().minutes());
            d.value.asTimestamp.second = static_cast<uint8_t>(v.asDateTime().seconds());
            if (v.asDateTime().has_fraction())
                d.value.asTimestamp.fsecond = v.asDateTime().nanoseconds();
			t = DPI_NATIVE_TYPE_TIMESTAMP;
			return true;
		case SQLDataType::Blob:
			{
				SAS_LOG_TRACE(logger, "dpiConn_newTempLob");
				auto ora_conn = conn->conn();
				SAS_LOG_ASSERT(logger, ora_conn, "oracle connection is not available");
				if (dpiConn_newTempLob(ora_conn, DPI_ORACLE_TYPE_BLOB, &d.value.asLOB) != DPI_SUCCESS)
				{
					auto err = ec.add(SAS_SQL__ERROR__CANNOT_BIND_PARAMETERS, getErrorText());
					SAS_LOG_ERROR(logger, err);
					return false;
				}
				t = DPI_NATIVE_TYPE_LOB;
			}
			return true;
		}

		auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "unsupported variant type: '"+std::to_string((int)v.type())+"'");
		SAS_LOG_ERROR(logger, err);
		return false;
	}

	bool toDataType(dpiNativeTypeNum t, SQLDataType & dt, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		switch (t)
		{
		case DPI_NATIVE_TYPE_INT64:
		case DPI_NATIVE_TYPE_UINT64:
			dt = SQLDataType::Number;
			return true;
		case DPI_NATIVE_TYPE_FLOAT:
		case DPI_NATIVE_TYPE_DOUBLE:
			dt = SQLDataType::Real;
			return true;
		case DPI_NATIVE_TYPE_BYTES:
			dt = SQLDataType::String;
			return true;
		case DPI_NATIVE_TYPE_TIMESTAMP:
			dt = SQLDataType::DateTime;
			return true;
		case DPI_NATIVE_TYPE_INTERVAL_DS:
		{
			auto err = ec.add(SAS_SQL__ERROR__NOT_SUPPORTED, "unknown data type: 'DPI_NATIVE_TYPE_INTERVAL_DS'");
			SAS_LOG_ERROR(logger, err);
			return false;
		}
		case DPI_NATIVE_TYPE_INTERVAL_YM:
		{
			auto err = ec.add(SAS_SQL__ERROR__NOT_SUPPORTED, "unknown data type: 'DPI_NATIVE_TYPE_INTERVAL_YM'");
			SAS_LOG_ERROR(logger, err);
			return false;
		}
		case DPI_NATIVE_TYPE_LOB:
			dt = SQLDataType::Blob;
			return true;
		case DPI_NATIVE_TYPE_OBJECT:
		{
			auto err = ec.add(SAS_SQL__ERROR__NOT_SUPPORTED, "unknown data type: 'DPI_NATIVE_TYPE_OBJECT'");
			SAS_LOG_ERROR(logger, err);
			return false;
		}
		case DPI_NATIVE_TYPE_STMT:
		{
			auto err = ec.add(SAS_SQL__ERROR__NOT_SUPPORTED, "unknown data type: 'DPI_NATIVE_TYPE_STMT'");
			SAS_LOG_ERROR(logger, err);
			return false;
		}
		case DPI_NATIVE_TYPE_BOOLEAN:
		{
			auto err = ec.add(SAS_SQL__ERROR__NOT_SUPPORTED, "unknown data type: 'DPI_NATIVE_TYPE_BOOLEAN'");
			SAS_LOG_ERROR(logger, err);
			return false;
		}
		case DPI_NATIVE_TYPE_ROWID:
		{
			auto err = ec.add(SAS_SQL__ERROR__NOT_SUPPORTED, "unknown data type: 'DPI_NATIVE_TYPE_ROWID'");
			SAS_LOG_ERROR(logger, err);
			return false;
		}
		}

        auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "unknown data type: '" + std::to_string(static_cast<int>(t)) + "'");
		SAS_LOG_ERROR(logger, err);
		return false;
	}

	bool toVariant(dpiNativeTypeNum t, dpiData * d, SQLVariant & v, ErrorCollector & ec)
	{
		SAS_LOG_NDC();

		if (d->isNull)
		{
			SQLDataType dt;
			if (!toDataType(t, dt, ec))
				return false;
			v = dt;
			return true;
		}

		switch (t)
		{
		case DPI_NATIVE_TYPE_INT64:
            v = SQLVariant(static_cast<long long>(d->value.asInt64));
			return true;
		case DPI_NATIVE_TYPE_UINT64:
            v = SQLVariant(static_cast<long long>(d->value.asUint64));
			return true;
		case DPI_NATIVE_TYPE_FLOAT:
            v = SQLVariant(static_cast<double>(d->value.asFloat));
			return true;
		case DPI_NATIVE_TYPE_DOUBLE:
            v = SQLVariant(static_cast<double>(d->value.asDouble));
			return true;
		case DPI_NATIVE_TYPE_BYTES:
			{
				std::string str;
				str.append(d->value.asBytes.ptr, d->value.asBytes.length);
				v = SQLVariant(str);
			}
			return true;
		case DPI_NATIVE_TYPE_TIMESTAMP:
			v = SQLVariant(SQLDateTime(
                static_cast<unsigned int>(d->value.asTimestamp.year),
                static_cast<unsigned int>(d->value.asTimestamp.month),
                static_cast<unsigned int>(d->value.asTimestamp.day),
                static_cast<unsigned int>(d->value.asTimestamp.hour),
                static_cast<unsigned int>(d->value.asTimestamp.minute),
                static_cast<unsigned int>(d->value.asTimestamp.second),
                static_cast<unsigned int>(d->value.asTimestamp.fsecond),
				d->value.asTimestamp.tzHourOffset,
                d->value.asTimestamp.tzMinuteOffset,
                false, 9), SQLVariant::DateTimeSubType::TimeStamp);
			return true;
		case DPI_NATIVE_TYPE_INTERVAL_DS:
		{
			auto err = ec.add(SAS_SQL__ERROR__NOT_SUPPORTED, "unknown data type: 'DPI_NATIVE_TYPE_INTERVAL_DS'");
			SAS_LOG_ERROR(logger, err);
			return false;
		}
		case DPI_NATIVE_TYPE_INTERVAL_YM:
		{
			auto err = ec.add(SAS_SQL__ERROR__NOT_SUPPORTED, "unknown data type: 'DPI_NATIVE_TYPE_INTERVAL_YM'");
			SAS_LOG_ERROR(logger, err);
			return false;
		}
		case DPI_NATIVE_TYPE_LOB:
			{
				uint64_t size;
				SAS_LOG_TRACE(logger, "dpiLob_getSize");
				if (dpiLob_getSize(d->value.asLOB, &size) != DPI_SUCCESS)
				{
					auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, getErrorText());
					SAS_LOG_ERROR(logger, err);
					return false;
				}

                std::vector<unsigned char> buff(static_cast<size_t>(size));
				if(size)
				{
					uint64_t r_size = size;
					SAS_LOG_TRACE(logger, "dpiLob_readBytes");
					if (dpiLob_readBytes(d->value.asLOB, 1, size, (char*)buff.data(), &r_size) != DPI_SUCCESS)
					{
						auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, getErrorText());
						SAS_LOG_ERROR(logger, err);
						return false;
					}

					if (r_size != size)
					{
						auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "not all blob has been read");
						SAS_LOG_ERROR(logger, err);
						return false;
					}
				}
				v = SQLVariant(buff);
			}

			return true;
		case DPI_NATIVE_TYPE_OBJECT:
		{
			auto err = ec.add(SAS_SQL__ERROR__NOT_SUPPORTED, "unknown data type: 'DPI_NATIVE_TYPE_OBJECT'");
			SAS_LOG_ERROR(logger, err);
			return false;
		}
		case DPI_NATIVE_TYPE_STMT:
		{
			auto err = ec.add(SAS_SQL__ERROR__NOT_SUPPORTED, "unknown data type: 'DPI_NATIVE_TYPE_STMT'");
			SAS_LOG_ERROR(logger, err);
			return false;
		}
		case DPI_NATIVE_TYPE_BOOLEAN:
		{
			auto err = ec.add(SAS_SQL__ERROR__NOT_SUPPORTED, "unknown data type: 'DPI_NATIVE_TYPE_BOOLEAN'");
			SAS_LOG_ERROR(logger, err);
			return false;
		}
		case DPI_NATIVE_TYPE_ROWID:
		{
			auto err = ec.add(SAS_SQL__ERROR__NOT_SUPPORTED, "unknown data type: 'DPI_NATIVE_TYPE_ROWID'");
			SAS_LOG_ERROR(logger, err);
			return false;
		}
		break;
		}

		auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "unknown data type: '" + std::to_string((int)t) + "'");
		SAS_LOG_ERROR(logger, err);
		return false;
	}
};

OraStatement::OraStatement(OraConnector * conn) : priv(new Priv(conn))
{ }

OraStatement::~OraStatement()
{
	SAS_LOG_NDC();
	priv->mut.lock();
//	priv->conn_mut.lock();
	SAS_LOG_TRACE(priv->conn->logger(), "mysql_stmt_close");
	if (priv->stmt)
	{
		SAS_LOG_TRACE(priv->logger, "dpiStmt_close");
        if (dpiStmt_close(priv->stmt, nullptr, 0) != DPI_SUCCESS)
		{
			SAS_LOG_ERROR(priv->logger, ErrorCollector::toString(SAS_SQL__ERROR__UNEXPECTED, priv->getErrorText()));
		}
	}
//	priv->conn_mut.unlock();
	priv->mut.unlock();
	delete priv;
}

bool OraStatement::prepare(const std::string & statement, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	std::unique_lock<std::mutex> __locker(priv->mut);
//	std::unique_lock<std::mutex> __locker_conn(priv->conn_mut);

	SAS_LOG_VAR(priv->conn->logger(), statement);

	if (priv->stmt)
	{
		SAS_LOG_TRACE(priv->logger, "dpiStmt_close");
        if (dpiStmt_close(priv->stmt, nullptr, 0) != DPI_SUCCESS)
		{
			auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, priv->getErrorText());
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}
		priv->stmt = nullptr;
	}

	dpiConn * conn;
	if (!(conn = priv->conn->conn(ec)))
		return false;

	SAS_LOG_TRACE(priv->logger, "dpiConn_prepareStmt");
    if (dpiConn_prepareStmt(conn, 0, statement.c_str(), static_cast<uint32_t>(statement.length()), nullptr, 0, &priv->stmt) != DPI_SUCCESS)
	{
		auto err = ec.add(SAS_SQL__ERROR__CANNOT_PREPARE_STATEMENT, priv->getErrorText());
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}

	SAS_LOG_ASSERT(priv->logger, priv->stmt, "stmt must be valid");

	return true;
}

unsigned long OraStatement::paramNum()
{
	SAS_LOG_NDC();
	std::unique_lock<std::mutex> __locker(priv->mut);
//	std::unique_lock<std::mutex> __locker_conn(priv->conn_mut);

	if (!priv->stmt)
	{
		SAS_LOG_ERROR(priv->logger, ErrorCollector::toString(SAS_SQL__ERROR__UNEXPECTED, "statemenet is not yet prepared"));
		return 0;
	}

	SAS_LOG_TRACE(priv->conn->logger(), "mysql_stmt_param_count");
	uint32_t cnt;
	if (dpiStmt_getBindCount(priv->stmt, &cnt) != DPI_SUCCESS)
	{
		SAS_LOG_ERROR(priv->logger, ErrorCollector::toString(SAS_SQL__ERROR__UNEXPECTED, priv->getErrorText()));
		return 0;
	}

	return cnt;
}

bool OraStatement::bindParam(const std::vector<SQLVariant> & params, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	std::unique_lock<std::mutex> __locker(priv->mut);
//	std::unique_lock<std::mutex> __locker_conn(priv->conn_mut);
	size_t params_size = params.size();

	if (!priv->stmt)
	{
		SAS_LOG_ERROR(priv->logger, ErrorCollector::toString(SAS_SQL__ERROR__UNEXPECTED, "statemenet is not yet prepared"));
		return 0;
	}

	SAS_LOG_TRACE(priv->conn->logger(), "mysql_stmt_param_count");
	uint32_t stmt_params_size;
	if (dpiStmt_getBindCount(priv->stmt, &stmt_params_size) != DPI_SUCCESS)
	{
		auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "statemenet is not yet prepared");
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}

	if (stmt_params_size > params_size)
	{
		SAS_LOG_VAR(priv->logger, stmt_params_size);
		SAS_LOG_VAR(priv->logger, params_size);
		auto err = ec.add(SAS_SQL__ERROR__CANNOT_BIND_PARAMETERS, "insufficient number of parameters");
		SAS_LOG_ERROR(priv->conn->logger(), err);
		return false;
	}
	else if(stmt_params_size != params_size)
	{
		SAS_LOG_VAR(priv->logger, stmt_params_size);
		SAS_LOG_VAR(priv->logger, params_size);
		SAS_LOG_WARN(priv->logger, "incorrect number of parameters to be bound");
	}

	bool has_error(false);
	for (size_t i(0); i < params_size; ++i)
	{
		dpiData d;
		dpiNativeTypeNum t;
		if (!priv->toOraData(params[i], d, t, ec))
			has_error = true;
		else
		{
			SAS_LOG_TRACE(priv->conn->logger(), "dpiStmt_bindValueByPos");
            if (dpiStmt_bindValueByPos(priv->stmt, static_cast<uint32_t>(i + 1), t, &d) != DPI_SUCCESS)
			{
				SAS_LOG_ERROR(priv->logger, priv->getErrorText());
				has_error = true;
			}
		}
	}

	return !has_error;
}

bool OraStatement::bindParam(const std::vector<std::pair<std::string /*name*/, SQLVariant>> & params, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	std::unique_lock<std::mutex> __locker(priv->mut);
//	std::unique_lock<std::mutex> __locker_conn(priv->conn_mut);
	size_t params_size = params.size();

	if (!priv->stmt)
	{
		auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "statemenet is not yet prepared");
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}

	bool has_error(false);
	dpiData d;
	dpiNativeTypeNum t;
	for (size_t i(0); i < params_size; ++i)
	{
		auto & p = params[i];
		if (!priv->toOraData(p.second, d, t, ec))
			has_error = true;
		else
		{
			SAS_LOG_TRACE(priv->conn->logger(), "dpiStmt_bindValueByPos");
            if (dpiStmt_bindValueByName(priv->stmt, p.first.c_str(), static_cast<uint32_t>(p.first.length()), t, &d) != DPI_SUCCESS)
			{
				auto err = ec.add(SAS_SQL__ERROR__CANNOT_BIND_PARAMETERS, priv->getErrorText());
				SAS_LOG_ERROR(priv->logger, err);
				has_error = true;
			}
		}
	}

	return !has_error;
}

bool OraStatement::execDML(ErrorCollector & ec)
{
	SAS_LOG_NDC();
	return exec(ec);
}

bool OraStatement::exec(ErrorCollector & ec)
{
	SAS_LOG_NDC();
	std::unique_lock<std::mutex> __locker(priv->mut);
//	std::unique_lock<std::mutex> __locker_conn(priv->conn_mut);

	if (!priv->stmt)
	{
		auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "statemenet is not yet prepared");
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}

	SAS_LOG_TRACE(priv->conn->logger(), "dpiStmt_execute");
	if (dpiStmt_execute(priv->stmt, DPI_MODE_EXEC_DEFAULT, &priv->query_cnt) != DPI_SUCCESS)
	{
		auto err = ec.add(SAS_SQL__ERROR__CANNOT_EXECUTE_STATEMENT, priv->getErrorText());
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}

	return true;
}

bool OraStatement::getLastGeneratedId(const std::string & schema, const std::string & table, const std::string & field, SQLVariant & ret, ErrorCollector & ec) // final
{
    (void)schema;
    (void)table;
    (void)field;
    (void)ret;
	SAS_LOG_NDC();

	auto err = ec.add(SAS_SQL__ERROR__NOT_SUPPORTED, "this functionality is not supported by oracle");
	SAS_LOG_ERROR(priv->logger, err);

	return false;
}

bool OraStatement::fieldNum(size_t & ret, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	std::unique_lock<std::mutex> __locker(priv->mut);

	if (!priv->stmt)
	{
		auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "statemenet is not yet prepared");
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}

	uint32_t _ret;
	SAS_LOG_TRACE(priv->conn->logger(), "dpiStmt_getNumQueryColumns");
	if (dpiStmt_getNumQueryColumns(priv->stmt, &_ret) != DPI_SUCCESS)
	{
		auto err = ec.add(SAS_SQL__ERROR__NO_META_INFO, priv->getErrorText());
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}
	ret = _ret;
	return true;
}

bool OraStatement::fields(std::vector<std::tuple<std::string /*db/scheme*/, std::string /*table*/, std::string /*field name*/, SQLDataType>> & ret, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	std::unique_lock<std::mutex> __locker(priv->mut);

	if (!priv->stmt)
	{
		auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "statemenet is not yet prepared");
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}

	//dpiStmtInfo info;
	//if (dpiStmt_getInfo(priv->stmt, &info) != DPI_SUCCESS())
	//{
	//	auto err = ec.add(SAS_SQL__ERROR__NO_META_INFO, priv->getErrorText());
	//	SAS_LOG_ERROR(priv->conn->logger(), err);
	//	return false;
	//}

	uint32_t cnt;
	SAS_LOG_TRACE(priv->conn->logger(), "dpiStmt_getNumQueryColumns");
	if (dpiStmt_getNumQueryColumns(priv->stmt, &cnt) != DPI_SUCCESS)
	{
		auto err = ec.add(SAS_SQL__ERROR__NO_META_INFO, priv->getErrorText());
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}
	ret.resize(cnt);

	dpiQueryInfo info;
	bool has_error(false);
	for (uint32_t i = 0; i < cnt; ++i)
	{
		if (dpiStmt_getQueryInfo(priv->stmt, i + 1, &info) != DPI_SUCCESS)
		{
			auto err = ec.add(SAS_SQL__ERROR__NO_META_INFO, priv->getErrorText());
			SAS_LOG_ERROR(priv->logger, err);
			has_error = true;
		}
		else
		{
			auto & f = ret[i];
			std::get<2>(f).append(info.name, info.nameLength);
			if (!priv->toDataType(info.typeInfo.defaultNativeTypeNum, std::get<3>(f), ec))
				has_error = true;
		}

	}

	return !has_error;
}

unsigned long long OraStatement::rowNum()
{
	SAS_LOG_NDC();
	std::unique_lock<std::mutex> __locker(priv->mut);

	if (!priv->stmt)
	{
		SAS_LOG_ERROR(priv->logger, ErrorCollector::toString(SAS_SQL__ERROR__UNEXPECTED, "statemenet is not yet prepared"));
		return 0;
	}

	uint64_t ret;
	SAS_LOG_TRACE(priv->conn->logger(), "dpiStmt_getRowCount");
	if (dpiStmt_getRowCount(priv->stmt, &ret) != DPI_SUCCESS)
	{
		SAS_LOG_ERROR(priv->logger, ErrorCollector::toString(SAS_SQL__ERROR__UNEXPECTED, priv->getErrorText()));
		return 0;
	}

	return ret;
}

bool OraStatement::fetch(std::vector<SQLVariant> & ret, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	std::unique_lock<std::mutex> __locker(priv->mut);

	if (!priv->stmt)
	{
		auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, "statemenet is not yet prepared");
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}

	int found = 0;
	uint32_t ri;
	SAS_LOG_TRACE(priv->logger, "dpiStmt_fetch");
	if (dpiStmt_fetch(priv->stmt, &found, &ri) != DPI_SUCCESS)
	{
		auto err = ec.add(SAS_SQL__ERROR__UNEXPECTED, priv->getErrorText());
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}

	if (!found)
	{
		auto err = ec.add(SAS_SQL__ERROR__NO_DATA, "(no data)");
		SAS_LOG_DEBUG(priv->logger, err);
		return false;
	}

	uint32_t cnt;
	SAS_LOG_TRACE(priv->logger, "dpiStmt_getNumQueryColumns");
	if (dpiStmt_getNumQueryColumns(priv->stmt, &cnt) != DPI_SUCCESS)
	{
		auto err = ec.add(SAS_SQL__ERROR__NO_META_INFO, priv->getErrorText());
		SAS_LOG_ERROR(priv->logger, err);
		return false;
	}

	dpiNativeTypeNum t;
	dpiData * d;
	ret.resize(cnt);
	bool has_error(false);
	for (uint32_t i(0); i < cnt; ++i)
	{
		SAS_LOG_TRACE(priv->logger, "dpiStmt_getQueryValue");
		if (dpiStmt_getQueryValue(priv->stmt, i + 1, &t, &d) != DPI_SUCCESS)
		{
			auto err = ec.add(SAS_SQL__ERROR__NO_META_INFO, priv->getErrorText());
			SAS_LOG_ERROR(priv->logger, err);
			has_error = true;
		}
		else if (!priv->toVariant(t, d, ret[i], ec))
			has_error = true;
	}

	return !has_error;
}

bool OraStatement::getSysDate(SAS::SQLDateTime & ret, ErrorCollector & ec)
{
	SAS_LOG_NDC();

	std::vector<SAS::SQLVariant> data;
	if (!prepare("select sysdate from dual", ec) || !exec(ec) || !fetch(data, ec))
		return false;

	SAS_LOG_ASSERT(priv->logger, data.size() == 1, "invalid length of data");
	ret = data[0].asDateTime();
	return true;
}

}
