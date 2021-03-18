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

#include "include/sasSQL/sqlvariant.h"
#include "include/sasSQL/sqldatetime.h"

#include <cstring>
#include <math.h>

namespace SAS {

struct SQLVariant::Priv
{
	Priv() : type(SQLDataType::None), dtSubType(SQLVariant::DateTimeSubType::None),
		isNull(false), number(0), real(0), buffer(nullptr), buffer_size(0)
	{ }

	SQLDataType type;
	SQLVariant::DateTimeSubType dtSubType;

	bool isNull;
	std::string string;
	long long number;
	double real;
	SQLDateTime datetime;
	std::vector<unsigned char> blob;
	unsigned char * buffer;
	size_t buffer_size;
};

SQLVariant::SQLVariant(const SQLVariant & o) : priv(new Priv(*o.priv))
{ }

SQLVariant::SQLVariant() : priv(new Priv)
{
	priv->type = SQLDataType::None;
	priv->isNull = true;
}

SQLVariant::SQLVariant(SQLDataType type, DateTimeSubType dtSubType) : priv(new Priv)
{
	priv->type = type;
	priv->dtSubType = dtSubType;
	priv->isNull = true;
}

SQLVariant::SQLVariant(const std::string & string, bool isNull) : priv(new Priv)
{
	priv->type = SQLDataType::String;
	priv->string = string;
	priv->isNull = isNull;
}

SQLVariant::SQLVariant(const char * string, bool isNull) : priv(new Priv)
{
	priv->type = SQLDataType::String;
	priv->string = string;
	priv->isNull = isNull;
}

SQLVariant::SQLVariant(long long number, bool isNull) : priv(new Priv)
{
	priv->type = SQLDataType::Number;
	priv->number = number;
	priv->isNull = isNull;
}

SQLVariant::SQLVariant(int number, bool isNull) : priv(new Priv)
{
	priv->type = SQLDataType::Number;
	priv->number = number;
	priv->isNull = isNull;
}

SQLVariant::SQLVariant(short number, bool isNull) : priv(new Priv)
{
	priv->type = SQLDataType::Number;
	priv->number = number;
	priv->isNull = isNull;
}

SQLVariant::SQLVariant(char number, bool isNull) : priv(new Priv)
{
	priv->type = SQLDataType::Number;
	priv->number = number;
	priv->isNull = isNull;
}

SQLVariant::SQLVariant(double real, bool isNull) : priv(new Priv)
{
	priv->type = SQLDataType::Real;
	priv->real = real;
	priv->isNull = isNull;
}

SQLVariant::SQLVariant(const SQLDateTime & datetime, DateTimeSubType dtSubType, bool isNull) : priv(new Priv)
{
	priv->type = SQLDataType::DateTime;
	if (dtSubType == DateTimeSubType::None)
	{
		if (!datetime.hours() && !datetime.minutes() && !datetime.seconds())
			priv->dtSubType = DateTimeSubType::Date;
		else if (!datetime.years() && !datetime.months() && !datetime.days())
			priv->dtSubType = datetime.has_msecs() ? DateTimeSubType::TimeStamp_Time : DateTimeSubType::Time;
		else
			priv->dtSubType = datetime.has_msecs() ? DateTimeSubType::TimeStamp : DateTimeSubType::DateTime;
	}
	else
		priv->dtSubType = dtSubType;

	priv->datetime = datetime;
	priv->isNull = isNull;
}

SQLVariant::SQLVariant(const std::vector<unsigned char> & blob, size_t size, bool isNull) : priv(new Priv)
{
	priv->type = SQLDataType::Blob;
	priv->blob.resize(size);
	std::memcpy(priv->blob.data(), blob.data(), size < blob.size() ? size : blob.size());
	priv->isNull = isNull;
}

SQLVariant::SQLVariant(const std::vector<unsigned char> & blob, bool isNull) : priv(new Priv)
{
	priv->type = SQLDataType::Blob;
	priv->blob = blob;
	priv->isNull = isNull;
}

SQLVariant::SQLVariant(unsigned char * buffer, size_t size, bool isNull) : priv(new Priv)
{
	priv->type = SQLDataType::Blob;
	priv->buffer = buffer;
	priv->buffer_size = size;
	priv->isNull = isNull;
}

SQLVariant::~SQLVariant()
{
	delete priv;
}

SQLVariant & SQLVariant::operator = (const SQLVariant & o)
{
	*priv = *o.priv;
	return *this;
}

SQLDataType SQLVariant::type() const
{
	return priv->type;
}

SQLVariant::DateTimeSubType SQLVariant::dtSubType() const
{
	return priv->dtSubType;
}

bool SQLVariant::isNull() const
{
	return priv->isNull;
}

const std::string & SQLVariant::asString() const
{
    auto that_priv = static_cast<Priv*>(priv);
	switch (priv->type)
	{
	case SQLDataType::None:
	case SQLDataType::String:
		break;
	case SQLDataType::Number:
		return that_priv->string = std::to_string(priv->number);
	case SQLDataType::Real:
		return that_priv->string = std::to_string(priv->real);
	case SQLDataType::DateTime:
		return that_priv->string = priv->datetime.toString();
	case SQLDataType::Blob:
		that_priv->string.clear();
		if (priv->buffer)
			that_priv->string.append((char *)priv->buffer, priv->buffer_size);
		else
			that_priv->string.append((char *)priv->blob.data(), priv->blob.size());
		break;
	}
	return priv->string;
}

const long long & SQLVariant::asNumber() const
{
    auto that_priv = static_cast<Priv*>(priv);
	switch (priv->type)
	{
	case SQLDataType::None:
	case SQLDataType::Number:
		break;
	case SQLDataType::String:
		try { return that_priv->number = std::stoll(priv->string); }
		catch (...) { break; }
	case SQLDataType::Real:
		return that_priv->number = (long long)priv->real;
	case SQLDataType::DateTime:
		return that_priv->number = priv->datetime.to_time_t();
	case SQLDataType::Blob:
		if (priv->buffer)
            that_priv->number = static_cast<long long>(priv->buffer_size);
		else
            that_priv->number = static_cast<long long>(priv->blob.size());
		break;
	}
	return priv->number;
}

const double & SQLVariant::asReal() const
{
	auto that_priv = (Priv*)priv;
	switch (priv->type)
	{
	case SQLDataType::None:
	case SQLDataType::Real:
		break;
	case SQLDataType::String:
		try { return that_priv->real = std::stod(priv->string); }
		catch (...) { break; }
	case SQLDataType::Number:
        return that_priv->real = static_cast<double>(priv->number);
	case SQLDataType::DateTime:
        return that_priv->real = static_cast<double>(priv->datetime.to_time_t());
	case SQLDataType::Blob:
		if (priv->buffer)
			that_priv->real = priv->buffer_size;
		else
			that_priv->real = priv->blob.size();
		break;
	}
	return priv->real;
}

const SQLDateTime & SQLVariant::asDateTime() const
{
	auto that_priv = (Priv*)priv;
	switch (priv->type)
	{
	case SQLDataType::None:
	case SQLDataType::DateTime:
		break;
	case SQLDataType::String:
		return that_priv->datetime = priv->string;
	case SQLDataType::Real:
        return that_priv->datetime = static_cast<time_t>(priv->real);
	case SQLDataType::Number:
        return that_priv->datetime = static_cast<time_t>(that_priv->number);
	case SQLDataType::Blob:
		break;
	}
	return priv->datetime;
}

unsigned char * SQLVariant::asBlob(size_t & size) const
{
	if (priv->buffer)
	{
		priv->buffer_size = size;
		return priv->buffer;
	}
	size = priv->blob.size();
	return priv->blob.data();
}

unsigned char SQLVariant::asBlobByte(size_t idx) const
{
	size_t size;
	auto bl = asBlob(size);
	if (idx >= size)
		return 0;
	return bl[idx];
}

std::string SQLVariant::toString() const
{
	if (priv->isNull)
		return "(null)";
	switch (priv->type)
	{
	case SQLDataType::None:
		return "(none)";
	case SQLDataType::Blob:
		return "(blob: " + std::to_string(priv->blob.size()) + ")";
	case SQLDataType::DateTime:
		return priv->datetime.toString();
	case SQLDataType::String:
		return priv->string;
	case SQLDataType::Real:
		return std::to_string(priv->real);
	case SQLDataType::Number:
		return std::to_string(priv->number);
	}
	return "(unknown)";
}

long long SQLVariant::toNumber() const
{
	if (priv->isNull)
		return 0;
	switch (priv->type)
	{
	case SQLDataType::None:
		return 0;
	case SQLDataType::Blob:
        return static_cast<long long>(priv->blob.size());
	case SQLDataType::DateTime:
		return priv->datetime.to_time_t();
	case SQLDataType::String:
		try
		{
			return std::stoll(priv->string);
		}
		catch (...)
		{
			return 0;
		}
	case SQLDataType::Real:
		return (long long)priv->real;
	case SQLDataType::Number:
		return priv->number;
	}
	return 0;
}

double SQLVariant::toReal() const
{
	if (priv->isNull)
		return 0;
	switch (priv->type)
	{
	case SQLDataType::None:
		return 0;
	case SQLDataType::Blob:
		return priv->blob.size();
	case SQLDataType::DateTime:
        return static_cast<double>(priv->datetime.to_time_t()) + (static_cast<double>(priv->datetime.fraction()) / ::pow(10, priv->datetime.precision()));
	case SQLDataType::String:
		try
		{
			return std::stod(priv->string);
		}
		catch (...)
		{
			return 0;
		}
	case SQLDataType::Real:
		return priv->real;
	case SQLDataType::Number:
        return static_cast<double>(priv->number);
	}
	return 0;
}

SQLDateTime SQLVariant::toDateTime() const
{
	if (priv->isNull)
		return SQLDateTime();
	switch (priv->type)
	{
	case SQLDataType::None:
	case SQLDataType::Blob:
		return SQLDateTime();
	case SQLDataType::DateTime:
		return priv->datetime;
	case SQLDataType::String:
		return SQLDateTime(priv->string);
	case SQLDataType::Real:
        return SQLDateTime(static_cast<time_t>(priv->real), static_cast<unsigned int>(priv->real - static_cast<long long>(priv->real) * 1000000000), 9);
	case SQLDataType::Number:
        return SQLDateTime(static_cast<time_t>(priv->number));
	}
	return SQLDateTime();
}

}
