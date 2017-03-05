/*
    This file is part of sasMySQL.

    sasMySQL is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasMySQL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasMySQL.  If not, see <http://www.gnu.org/licenses/>
 */

#include "mysqlstatement.h"
#include "mysqlconnector.h"
#include "mysqlresult.h"

#include <sasCore/errorcollector.h>
#include <sasSQL/sqldatetime.h>

#include <string.h>
#include <memory>
#include <utility>
#include <mysql/mysql_time.h>
#include <assert.h>

namespace SAS {

struct MySQLStatement_priv
{
	MySQLStatement_priv(MYSQL_STMT * stmt_, MySQLConnector * conn_) : conn(conn_), conn_mut(conn_->mutex()), stmt(stmt_)
	{ }


	struct BaseResultHelper
	{
		inline BaseResultHelper() : out_size(0), is_null(1), is_error(0) { }
		virtual inline ~BaseResultHelper() { }

		virtual SQLVariant getData() const = 0;

		unsigned long out_size;
		my_bool is_null;
		my_bool is_error;
	};

	template<typename N_Type>
	struct N_ResultHelper : public BaseResultHelper
	{
		typedef N_Type Type;
		inline N_ResultHelper() : BaseResultHelper()
		{
			memset(&data, 0, data_size = sizeof(N_Type));
		}

		virtual inline ~N_ResultHelper()
		{ }

		static N_ResultHelper<N_Type> * init(void *& buffer)
		{
			auto ret = new N_ResultHelper<N_Type>();
			buffer = &ret->data;
			return ret;
		}

		virtual SQLVariant getData() const final
		{
			return SQLVariant(data, is_null!=FALSE);
		}
	private:
		unsigned long data_size;
		N_Type data;
	};

	struct BLOB_ResultHelper : public BaseResultHelper
	{
		inline BLOB_ResultHelper(unsigned long size) : BaseResultHelper(), buffer(size)
		{ }

		virtual inline ~BLOB_ResultHelper()
		{ }

		static BLOB_ResultHelper * init(unsigned long size, void *& buffer)
		{
			auto ret = new BLOB_ResultHelper(size);
			buffer = ret->buffer.data();
			return ret;
		}

		static BLOB_ResultHelper * init(unsigned long size, void *& buffer, unsigned long & buffer_size)
		{
			auto ret = new BLOB_ResultHelper(size);
			buffer = ret->buffer.data();
			buffer_size = ret->buffer.size();
			return ret;
		}

		virtual SQLVariant getData() const final
		{
			if(is_null)
				return SQLVariant(SQLDataType::Blob);

			return SQLVariant(buffer, (size_t)out_size);
		}
	private:
		std::vector<unsigned char> buffer;
	};

	struct Str_ResultHelper : public BaseResultHelper
	{
		Str_ResultHelper(unsigned long size) : buffer(size+1)
		{ }

		virtual ~Str_ResultHelper()
		{ }

		static Str_ResultHelper * init(unsigned long size, void *& buffer)
		{
			auto ret = new Str_ResultHelper(size);
			buffer = ret->buffer.data();
			return ret;
		}

		static Str_ResultHelper * init(unsigned long size, void *& buffer, unsigned long & buffer_size)
		{
			auto ret = new Str_ResultHelper(size);
			buffer = ret->buffer.data();
			buffer_size = ret->buffer.size()-1;
			return ret;
		}

		virtual SQLVariant getData() const final
		{
			if(is_null)
				return SQLVariant(SQLDataType::String);

			return SQLVariant(buffer.data());
		}
	private:
		std::vector<char> buffer;
	};

	struct DateTime_ResultHelper : public BaseResultHelper
	{
		inline DateTime_ResultHelper() : BaseResultHelper()
		{
			memset(&data, 0, data_size = sizeof(MYSQL_TIME));
		}

		virtual inline ~DateTime_ResultHelper() { }

		static DateTime_ResultHelper * init(void *& buffer)
		{
			auto ret = new DateTime_ResultHelper();
			buffer = &ret->data;
			return ret;
		}

		virtual SQLVariant getData() const
		{
			SQLVariant::DateTimeSubType sub_type(SQLVariant::DateTimeSubType::None);
			switch(data.time_type)
			{
			case MYSQL_TIMESTAMP_NONE:
			case MYSQL_TIMESTAMP_ERROR:
				break;
			case MYSQL_TIMESTAMP_DATE:
				sub_type = SQLVariant::DateTimeSubType::Date;
				break;
			case MYSQL_TIMESTAMP_DATETIME:
				sub_type = SQLVariant::DateTimeSubType::DateTime;
				break;
			case MYSQL_TIMESTAMP_TIME:
				sub_type = SQLVariant::DateTimeSubType::Time;
				break;
			}

			if(is_null)
				return SQLVariant(SQLDataType::DateTime, sub_type);

			return SQLVariant(SQLDateTime(data.year, data.month, data.day,
				data.hour, data.minute, data.second, data.second_part, false, data.neg!=FALSE), sub_type);
		}

	private:
		MYSQL_TIME data;
		unsigned long data_size;
	};

	std::vector<std::shared_ptr<BaseResultHelper>> res_helpers;
	std::vector<MYSQL_BIND> res_binders;

	MySQLConnector * conn;
	std::mutex & conn_mut;
	std::mutex mut;
	MYSQL_STMT * stmt;

	struct BaseParamHelper
	{
		virtual inline ~BaseParamHelper() { }
		SQLVariant var;
		MYSQL_TIME t;
	};

	template<typename C_Type>
	struct ParamHelper : public BaseParamHelper
	{
		typedef C_Type Type;
		inline ParamHelper(const Type & d) : data(d)
		{ }

		inline ParamHelper()
		{
			memset(&data, 0, sizeof(Type));
		}

		virtual inline ~ParamHelper() { }

		Type data;
	};

	std::vector<std::shared_ptr<BaseParamHelper>> param_helpers;
	std::vector<MYSQL_BIND> param_binders;
};

MySQLStatement::MySQLStatement(MYSQL_STMT * stmt, MySQLConnector * conn) : priv(new MySQLStatement_priv(stmt, conn))
{ }

MySQLStatement::~MySQLStatement()
{
	std::unique_lock<std::mutex> __locker(priv->conn_mut);
	mysql_stmt_close(priv->stmt);
	delete priv;
}

bool MySQLStatement::prepare(const std::string & statement, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	std::unique_lock<std::mutex> __locker(priv->conn_mut);

	SAS_LOG_VAR(priv->conn->logger(), statement);

	SAS_LOG_TRACE(priv->conn->logger(), "free buffers");
	priv->param_binders.clear();
	priv->param_helpers.clear();
	priv->res_binders.clear();
	priv->res_helpers.clear();

	SAS_LOG_TRACE(priv->conn->logger(), "mysql_stmt_free_result");
	mysql_stmt_free_result(priv->stmt);

	SAS_LOG_TRACE(priv->conn->logger(), "mysql_stmt_prepare");
	if(mysql_stmt_prepare(priv->stmt, statement.c_str(), statement.length()))
	{
		auto err = ec.add(-1, std::string("could not prepare SQL statement: ") + mysql_stmt_error(priv->stmt));
		SAS_LOG_ERROR(priv->conn->logger(), err);
		return false;
	}

	return true;
}

unsigned long MySQLStatement::paramNum()
{
	SAS_LOG_NDC();
	std::unique_lock<std::mutex> __locker(priv->conn_mut);
	SAS_LOG_TRACE(priv->conn->logger(), "mysql_stmt_param_count");
	return mysql_stmt_param_count(priv->stmt);
}

bool MySQLStatement::bindParam(const std::vector<SQLVariant> & params, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	std::unique_lock<std::mutex> __locker(priv->conn_mut);
	size_t params_size = params.size();

	SAS_LOG_TRACE(priv->conn->logger(), "mysql_stmt_param_count");
	unsigned long stmt_params_size;
	if((stmt_params_size = mysql_stmt_param_count(priv->stmt)) > params_size)
	{
		SAS_LOG_VAR(priv->conn->logger(), stmt_params_size);
		SAS_LOG_VAR(priv->conn->logger(), params_size);
		auto err = ec.add(-1, "insufficient number of parameters");
		SAS_LOG_ERROR(priv->conn->logger(), err);
		return false;
	}

	priv->param_helpers.resize(params_size);
	priv->param_binders.resize(params_size);

	bool has_error(false);
	for(size_t i = 0, l = stmt_params_size; i < l; ++i)
	{
		auto & b = priv->param_binders[i];
		auto & v = priv->param_helpers[i];
		auto & p = params[i];
		b.buffer_type = MYSQL_TYPE_NULL;
		switch(p.type())
		{
		case SQLDataType::None:
		{
			auto err = ec.add(-1, std::string("type of parameter '") + std::to_string(i) + std::string("' is not specified"));
			SAS_LOG_ERROR(priv->conn->logger(), err);
			has_error = true;
			break;
		}
		case SQLDataType::String:
			b.buffer_type = MYSQL_TYPE_VAR_STRING;
			if(!p.isNull())
			{
				auto _v = new MySQLStatement_priv::ParamHelper<std::string>(p.asString());
				v.reset(_v);
				b.buffer = (void*)_v->data.c_str();
				b.buffer_length = _v->data.length();
			}
			else
				b.is_null_value = 0;
			break;
		case SQLDataType::Number:
			b.buffer_type = MYSQL_TYPE_LONGLONG;
			if(!p.isNull())
			{
				auto _v = new MySQLStatement_priv::ParamHelper<long long>(p.asNumber());
				v.reset(_v);
				b.buffer = (void*)&_v->data;
				b.buffer_length = sizeof(long long);
			}
			else
				b.is_null_value = 0;
			break;
		case SQLDataType::Real:
			b.buffer_type = MYSQL_TYPE_DOUBLE;
			if(!p.isNull())
			{
				auto _v = new MySQLStatement_priv::ParamHelper<double>(p.asReal());
				v.reset(_v);
				b.buffer = (void*)&_v->data;
				b.buffer_length = sizeof(double);
			}
			else
				b.is_null_value = 0;
			break;
		case SQLDataType::DateTime:
			b.buffer_type = MYSQL_TYPE_NULL;
			switch(p.dtSubType())
			{
			case SQLVariant::DateTimeSubType::None:
			{
				auto err = ec.add(-1, std::string("date time sub type of parameter '") + std::to_string(i) + std::string("' is not specified"));
				SAS_LOG_ERROR(priv->conn->logger(), err);
				has_error = true;
				break;
			}
			case SQLVariant::DateTimeSubType::Date:
				b.buffer_type = MYSQL_TYPE_DATE;
				break;
			case SQLVariant::DateTimeSubType::Time:
				b.buffer_type = MYSQL_TYPE_TIME;
				break;
			case SQLVariant::DateTimeSubType::DateTime:
				b.buffer_type = MYSQL_TYPE_DATETIME;
				break;
			case SQLVariant::DateTimeSubType::TimeStamp:
			case SQLVariant::DateTimeSubType::TimeStamp_Date:
			case SQLVariant::DateTimeSubType::TimeStamp_Time:
				b.buffer_type = MYSQL_TYPE_TIMESTAMP;
				break;
			}
			if(b.buffer_type == MYSQL_TYPE_NULL)
			{
				auto err = ec.add(-1, std::string("unsupported date time sub type of parameter '") + std::to_string(i) + std::string("'"));
				SAS_LOG_ERROR(priv->conn->logger(), err);
				has_error = true;
				break;
			}

			if(!p.isNull())
			{
				auto _v = new MySQLStatement_priv::ParamHelper<MYSQL_TIME>();
				v.reset(_v);

				auto & dt = p.asDateTime();

				_v->data.year = dt.years();
				_v->data.month = dt.months();
				_v->data.day = dt.days();
				_v->data.hour = dt.hours();
				_v->data.minute = dt.minutes();
				_v->data.second = dt.seconds();
				_v->data.second_part = dt.msecs();
				_v->data.neg = dt.negative();

				_v->data.time_type = MYSQL_TIMESTAMP_ERROR;
				switch(p.dtSubType())
				{
				case SQLVariant::DateTimeSubType::None:
					break;
				case SQLVariant::DateTimeSubType::Date:
				case SQLVariant::DateTimeSubType::Time:
				case SQLVariant::DateTimeSubType::DateTime:
					_v->data.time_type = MYSQL_TIMESTAMP_NONE;
					break;
				case SQLVariant::DateTimeSubType::TimeStamp:
					_v->data.time_type = MYSQL_TIMESTAMP_DATETIME;
					break;
				case SQLVariant::DateTimeSubType::TimeStamp_Date:
					_v->data.time_type = MYSQL_TIMESTAMP_DATE;
					break;
				case SQLVariant::DateTimeSubType::TimeStamp_Time:
					_v->data.time_type = MYSQL_TIMESTAMP_TIME;
					break;
				}
				b.buffer = &_v->data;
				b.buffer_length = sizeof(MYSQL_TIME);
			}
			else
				b.is_null_value = 0;
			break;
		case SQLDataType::Blob:
			b.buffer_type = MYSQL_TYPE_BLOB;
			if(!p.isNull())
			{
				auto _v = new MySQLStatement_priv::ParamHelper<SQLVariant>(p);
				v.reset(_v);
				size_t tmp_size;
				b.buffer = _v->data.asBlob(tmp_size);
				b.buffer_length = tmp_size;
			}
			else
				b.is_null_value = 0;
			break;
		}
		if(b.buffer_type == MYSQL_TYPE_NULL)
		{
			auto err = ec.add(-1, std::string("unsupported type of parameter '") + std::to_string(i) + std::string("'"));
			SAS_LOG_ERROR(priv->conn->logger(), err);
			has_error = true;
			break;
		}
	}
	if(has_error)
		return false;
	SAS_LOG_TRACE(priv->conn->logger(), "mysql_stmt_bind_param");
	if(mysql_stmt_bind_param(priv->stmt, priv->param_binders.data()))
	{
		auto err = ec.add(-1, std::string("could not bind parameters: ") + mysql_stmt_error(priv->stmt));
		SAS_LOG_ERROR(priv->conn->logger(), err);
		return false;
	}

	return true;
}

bool MySQLStatement::execDML(ErrorCollector & ec)
{
	SAS_LOG_NDC();
	std::unique_lock<std::mutex> __locker(priv->conn_mut);

	SAS_LOG_TRACE(priv->conn->logger(), "mysql_stmt_execute");
	if(mysql_stmt_execute(priv->stmt))
	{
		auto err = ec.add(-1, std::string("could not execute statement: ") + mysql_stmt_error(priv->stmt));
		SAS_LOG_ERROR(priv->conn->logger(), err);
		return false;
	}

	return true;
}

bool MySQLStatement::exec(ErrorCollector & ec)
{
	SAS_LOG_NDC();
	std::unique_lock<std::mutex> __locker(priv->conn_mut);

	MYSQL_RES * meta_res;
	SAS_LOG_TRACE(priv->conn->logger(), "mysql_stmt_result_metadata");
	if(!(meta_res =  mysql_stmt_result_metadata(priv->stmt)))
	{
		auto err = ec.add(-1, std::string("no meta information is found: ") + mysql_stmt_error(priv->stmt));
		SAS_LOG_ERROR(priv->conn->logger(), err);
		return false;
	}

	SAS_LOG_TRACE(priv->conn->logger(), "mysql_num_fields");
	unsigned long fields_num = mysql_num_fields(meta_res);
	SAS_LOG_VAR(priv->conn->logger(), fields_num);

	priv->res_helpers.resize(fields_num);
	priv->res_binders.resize(fields_num);

	bool has_error(false);
	for(unsigned long i = 0; i < fields_num; ++i)
	{
		MYSQL_FIELD * f = mysql_fetch_field_direct(meta_res, i);
		auto & b = priv->res_binders[i];
		auto & h = priv->res_helpers[i];
		b.buffer_type = f->type;
		switch(f->type)
		{
		case MYSQL_TYPE_NULL:
			break;

		case MYSQL_TYPE_TINY:
			h.reset(MySQLStatement_priv::N_ResultHelper<char>::init(b.buffer));
			break;
		case MYSQL_TYPE_LONG:
		case MYSQL_TYPE_INT24:
			h .reset(MySQLStatement_priv::N_ResultHelper<int>::init(b.buffer));
			break;
		case MYSQL_TYPE_LONGLONG:
			h.reset(MySQLStatement_priv::N_ResultHelper<long long>::init(b.buffer));
			break;
		case MYSQL_TYPE_FLOAT:
			h.reset(MySQLStatement_priv::N_ResultHelper<float>::init(b.buffer));
			break;
		case MYSQL_TYPE_DOUBLE:
			h.reset(MySQLStatement_priv::N_ResultHelper<double>::init(b.buffer));
			break;
		case MYSQL_TYPE_VARCHAR:
		case MYSQL_TYPE_VAR_STRING:
		case MYSQL_TYPE_STRING:
		case MYSQL_TYPE_NEWDECIMAL:
		case MYSQL_TYPE_DECIMAL:
			h.reset(MySQLStatement_priv::Str_ResultHelper::init(
					priv->conn->settings().max_buffer_size > f->length ? f->length : priv->conn->settings().max_buffer_size,
					b.buffer, b.buffer_length));
			break;
		case MYSQL_TYPE_TINY_BLOB:
		case MYSQL_TYPE_MEDIUM_BLOB:
		case MYSQL_TYPE_LONG_BLOB:
		case MYSQL_TYPE_BLOB:
		case MYSQL_TYPE_BIT:
			h.reset(MySQLStatement_priv::BLOB_ResultHelper::init(
					priv->conn->settings().max_buffer_size > f->length ? f->length : priv->conn->settings().max_buffer_size,
					b.buffer, b.buffer_length));
			break;
		case MYSQL_TYPE_YEAR:
		case MYSQL_TYPE_SHORT:
			h.reset(MySQLStatement_priv::N_ResultHelper<short>::init(b.buffer));
			break;
		case MYSQL_TYPE_NEWDATE:
		case MYSQL_TYPE_DATE:
		case MYSQL_TYPE_TIME:
		case MYSQL_TYPE_TIME2:
		case MYSQL_TYPE_TIMESTAMP:
		case MYSQL_TYPE_TIMESTAMP2:
		case MYSQL_TYPE_DATETIME:
		case MYSQL_TYPE_DATETIME2:
			h.reset(MySQLStatement_priv::DateTime_ResultHelper::init(b.buffer));
			break;
		case MYSQL_TYPE_JSON:
		case MYSQL_TYPE_ENUM:
		case MYSQL_TYPE_SET:
		case MYSQL_TYPE_GEOMETRY:
			{
				auto err = ec.add(-1, "unsupported result type:" + f->type);
				SAS_LOG_ERROR(priv->conn->logger(), err);
				has_error = true;
				break;
			}
		}
		b.length = &h->out_size;
		b.error = &h->is_error;
		b.is_null = &h->is_null;
	}
	if(has_error)
		return false;

	SAS_LOG_TRACE(priv->conn->logger(), "mysql_stmt_bind_result");
	if(mysql_stmt_bind_result(priv->stmt, priv->res_binders.data()))
	{
		auto err = ec.add(-1, std::string("could not bind results: ") + mysql_stmt_error(priv->stmt));
		SAS_LOG_ERROR(priv->conn->logger(), err);
		return false;
	}

	SAS_LOG_TRACE(priv->conn->logger(), "mysql_stmt_execute");
	if(mysql_stmt_execute(priv->stmt))
	{
		auto err = ec.add(-1, std::string("could not execute statement: ") + mysql_stmt_error(priv->stmt));
		SAS_LOG_ERROR(priv->conn->logger(), err);
		return false;
	}

	SAS_LOG_TRACE(priv->conn->logger(), "mysql_stmt_store_result");
	if(mysql_stmt_store_result(priv->stmt))
	{
		auto err = ec.add(-1, std::string("could not store statement result: ") + mysql_stmt_error(priv->stmt));
		SAS_LOG_ERROR(priv->conn->logger(), err);
		return false;
	}

	return true;
}

bool MySQLStatement::fieldNum(size_t & ret, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	std::unique_lock<std::mutex> __locker(priv->mut);
	MYSQL_RES * meta_res;
	SAS_LOG_TRACE(priv->conn->logger(), "mysql_stmt_result_metadata");
	if(!(meta_res =  mysql_stmt_result_metadata(priv->stmt)))
	{
		auto err = ec.add(-1, std::string("no meta information is found: ") + mysql_stmt_error(priv->stmt));
		SAS_LOG_ERROR(priv->conn->logger(), err);
		return false;
	}

	SAS_LOG_TRACE(priv->conn->logger(), "mysql_num_fields");
	ret = mysql_num_fields(meta_res);
	return true;
}

bool MySQLStatement::fields(std::vector<std::tuple<std::string /*db/scheme*/, std::string /*table*/, std::string /*field name*/, SQLDataType>> & ret, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	std::unique_lock<std::mutex> __locker(priv->mut);

	MYSQL_RES * meta_res;
	SAS_LOG_TRACE(priv->conn->logger(), "mysql_stmt_result_metadata");
	if(!(meta_res =  mysql_stmt_result_metadata(priv->stmt)))
	{
		auto err = ec.add(-1, std::string("no meta information is found: ") + mysql_stmt_error(priv->stmt));
		SAS_LOG_ERROR(priv->conn->logger(), err);
		return false;
	}

	SAS_LOG_TRACE(priv->conn->logger(), "mysql_num_fields");
	unsigned long fields_num = mysql_num_fields(meta_res);
	SAS_LOG_VAR(priv->conn->logger(), fields_num);
	ret.resize(fields_num);

	bool has_error(false);
	for(unsigned long i = 0; i < fields_num; ++i)
	{
		MYSQL_FIELD * f;
		assert(f = mysql_fetch_field_direct(meta_res, i));

		ret[i] = std::tuple<std::string, std::string, std::string, SQLDataType>(
			f->db ? f->db : std::string(),
			f->table ? f->table : std::string(),
			f->name ? f->name : std::string(),
			MySQLResult::toDataType(f->type));
	}

	return !has_error;
}

unsigned long long MySQLStatement::rowNum()
{
	SAS_LOG_NDC();
	std::unique_lock<std::mutex> __locker(priv->mut);
	SAS_LOG_TRACE(priv->conn->logger(), "mysql_stmt_num_rows");
	return mysql_stmt_num_rows(priv->stmt);
}

bool MySQLStatement::fetch(std::vector<SQLVariant> & ret, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	std::unique_lock<std::mutex> __locker(priv->mut);

	switch(mysql_stmt_fetch(priv->stmt))
	{
	case 0:
		break;
	case MYSQL_REPORT_DATA_TRUNCATION:
		SAS_LOG_ERROR(priv->conn->logger(), "mysql_stmt_fetch has returned with MYSQL_REPORT_DATA_TRUNCATION");
		break;
	case MYSQL_NO_DATA:
		return false;
	}

	ret.resize(priv->res_helpers.size());
	for(size_t i(0), l(ret.size()); i < l; ++i)
		ret[i] = priv->res_helpers[i]->getData();

	return true;
}

}
