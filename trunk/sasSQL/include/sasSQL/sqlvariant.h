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

#ifndef INCLUDE_SASSQL_SQLVARIANT_H_
#define INCLUDE_SASSQL_SQLVARIANT_H_

#include <sasCore/defines.h>
#include "config.h"

#include <string>
#include <vector>

namespace SAS {

class SQLDateTime;

enum class SQLDataType
{
	None, String, Number, Real, DateTime, Blob
};

class SAS_SQL__CLASS SQLVariant
{
	struct Priv;
	Priv * priv;
public:
	enum class DateTimeSubType
	{
		None, Date, Time, DateTime, TimeStamp, TimeStamp_Date, TimeStamp_Time
	};

	SQLVariant(const SQLVariant & o);
	SQLVariant();
	SQLVariant(SQLDataType type, DateTimeSubType dtSubType = DateTimeSubType::None);
	SQLVariant(const std::string & string, bool isNull = false);
	SQLVariant(const char * string, bool isNull = false);
	SQLVariant(long long number, bool isNull = false);
	SQLVariant(int number, bool isNull = false);
	SQLVariant(short number, bool isNull = false);
	SQLVariant(char number, bool isNull = false);
	SQLVariant(double real, bool isNull = false);
	SQLVariant(const SQLDateTime & datetime, DateTimeSubType dtSubType, bool isNull = false);
	SQLVariant(const std::vector<unsigned char> & blob, size_t size, bool isNull = false);
	SQLVariant(const std::vector<unsigned char> & blob, bool isNull = false);
	SQLVariant(unsigned char * buffer, size_t size, bool isNull = false);
	virtual ~SQLVariant();

	SQLVariant & operator = (const SQLVariant & o);

	SQLDataType type() const;
	DateTimeSubType dtSubType() const;
	bool isNull() const;

	const std::string & asString() const;
	const long long & asNumber() const;
	const double & asReal() const;
	const SQLDateTime & asDateTime() const;
	unsigned char * asBlob(size_t & s) const;
	unsigned char asBlobByte(size_t idx) const;

	std::string toString() const;
};

}

#endif /* INCLUDE_SASSQL_SQLVARIANT_H_ */
