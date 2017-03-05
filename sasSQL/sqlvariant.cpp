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
    along with ${project_name}.  If not, see <http://www.gnu.org/licenses/>
 */

#include "include/sasSQL/sqlvariant.h"
#include "include/sasSQL/sqldatetime.h"

#include <string.h>

namespace SAS {

struct SQLVariant_priv
{
	SQLVariant_priv() : type(SQLDataType::None), dtSubType(SQLVariant::DateTimeSubType::None),
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

SQLVariant::SQLVariant(const SQLVariant & o) : priv(new SQLVariant_priv(*o.priv))
{ }

SQLVariant::SQLVariant() : priv(new SQLVariant_priv)
{
	priv->type = SQLDataType::None;
	priv->isNull = true;
}

SQLVariant::SQLVariant(SQLDataType type, DateTimeSubType dtSubType) : priv(new SQLVariant_priv)
{
	priv->type = type;
	priv->isNull = true;
}

SQLVariant::SQLVariant(const std::string & string, bool isNull) : priv(new SQLVariant_priv)
{
	priv->type = SQLDataType::String;
	priv->string = string;
	priv->isNull = isNull;
}

SQLVariant::SQLVariant(const char * string, bool isNull) : priv(new SQLVariant_priv)
{
	priv->type = SQLDataType::String;
	priv->string = string;
	priv->isNull = isNull;
}

SQLVariant::SQLVariant(long long number, bool isNull) : priv(new SQLVariant_priv)
{
	priv->type = SQLDataType::Number;
	priv->number = number;
	priv->isNull = isNull;
}

SQLVariant::SQLVariant(int number, bool isNull) : priv(new SQLVariant_priv)
{
	priv->type = SQLDataType::Number;
	priv->number = number;
	priv->isNull = isNull;
}

SQLVariant::SQLVariant(short number, bool isNull) : priv(new SQLVariant_priv)
{
	priv->type = SQLDataType::Number;
	priv->number = number;
	priv->isNull = isNull;
}

SQLVariant::SQLVariant(char number, bool isNull) : priv(new SQLVariant_priv)
{
	priv->type = SQLDataType::Number;
	priv->number = number;
	priv->isNull = isNull;
}

SQLVariant::SQLVariant(double real, bool isNull) : priv(new SQLVariant_priv)
{
	priv->type = SQLDataType::Real;
	priv->real = real;
	priv->isNull = isNull;
}

SQLVariant::SQLVariant(const SQLDateTime & datetime, DateTimeSubType dtSubType, bool isNull) : priv(new SQLVariant_priv)
{
	priv->type = SQLDataType::DateTime;
	if(dtSubType == DateTimeSubType::None)
	{
		if(!datetime.hours() && !datetime.minutes() && !datetime.seconds())
			priv->dtSubType = DateTimeSubType::Date;
		else if(!datetime.years() && !datetime.months() && !datetime.days())
			priv->dtSubType = datetime.has_msecs() ? DateTimeSubType::TimeStamp_Time : DateTimeSubType::Time;
		else
			priv->dtSubType = datetime.has_msecs() ? DateTimeSubType::TimeStamp : DateTimeSubType::DateTime;
	}
	else
		priv->dtSubType = dtSubType;

	priv->datetime = datetime;
	priv->isNull = isNull;
}

SQLVariant::SQLVariant(const std::vector<unsigned char> & blob, size_t size, bool isNull) : priv(new SQLVariant_priv)
{
	priv->type = SQLDataType::Blob;
	priv->blob.resize(size);
	memcpy(priv->blob.data(), blob.data(), size < blob.size() ? size : blob.size());
	priv->isNull = isNull;
}

SQLVariant::SQLVariant(const std::vector<unsigned char> & blob, bool isNull) : priv(new SQLVariant_priv)
{
	priv->type = SQLDataType::Blob;
	priv->blob = blob;
	priv->isNull = isNull;
}

SQLVariant::SQLVariant(unsigned char * buffer, size_t size, bool isNull) : priv(new SQLVariant_priv)
{
	priv->type = SQLDataType::Blob;
	priv->buffer  = buffer;
	priv->buffer_size  = size;
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
	return priv->string;
}

const long long & SQLVariant::asNumber() const
{
	return priv->number;
}

const double & SQLVariant::asReal() const
{
	return priv->real;
}

const SQLDateTime & SQLVariant::asDateTime() const
{
	return priv->datetime;
}

unsigned char * SQLVariant::asBlob(size_t & size) const
{
	if(priv->buffer)
	{
		priv->buffer_size = size;
		return priv->buffer;
	}
	size = priv->blob.size();
	return priv->blob.data();
}

std::string SQLVariant::toString() const
{
	if(priv->isNull)
		return "(null)";
	switch(priv->type)
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

}
