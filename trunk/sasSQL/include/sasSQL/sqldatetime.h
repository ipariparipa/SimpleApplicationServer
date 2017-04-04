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

#ifndef INCLUDE_SASSQL_SQLDATETIME_H_
#define INCLUDE_SASSQL_SQLDATETIME_H_

#include <sasCore/defines.h>
#include "config.h"
#include <ctime>
#include <string>

namespace SAS {

struct SQLDateTime_priv;
class SAS_SQL__CLASS SQLDateTime
{
public:
	SQLDateTime(const SQLDateTime & o);
	SQLDateTime();
	SQLDateTime(time_t t);
	SQLDateTime(const tm * t);
	SQLDateTime(unsigned int year, unsigned int months, unsigned int days, unsigned int hours, unsigned int minutes, unsigned int seconds, int msecs = -1, bool daylightSaveTime = false, bool negative = false, short ms_precision = 6);
	virtual ~SQLDateTime();

	SQLDateTime & operator = (const SQLDateTime & o);

	bool isNull() const;
	bool has_msecs() const;

	unsigned int years() const;
	unsigned int months() const;
	unsigned int days() const;
	unsigned int hours() const;
	unsigned int minutes() const;
	unsigned int seconds() const;
	int msecs() const;
	bool daylightSaveTime() const;
	bool negative() const;

	std::string toString() const;

	tm to_tm() const;
	time_t to_time_t() const;

private:
	SQLDateTime_priv * priv;
};

}

#endif /* INCLUDE_SASSQL_SQLDATETIME_H_ */