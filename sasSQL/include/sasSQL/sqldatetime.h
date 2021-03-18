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
#include <chrono>

namespace SAS {

class SAS_SQL__CLASS SQLDateTime
{
	struct Priv;
	Priv * priv;
public:
	SQLDateTime(const SQLDateTime & o);
	SQLDateTime();
	SQLDateTime(time_t t);
    SQLDateTime(time_t t, unsigned int fraction, int precision = 6);
    SQLDateTime(std::chrono::system_clock::time_point tp, int precision = 9);
    SQLDateTime(const tm * t);
    SQLDateTime(const tm * t, unsigned int fraction, int precision = 6);
    SQLDateTime(unsigned int years, unsigned int months, unsigned int days, unsigned int hours, unsigned int minutes, unsigned int seconds, unsigned int fraction = 0, bool negative = false, int precision = 0);
    SQLDateTime(unsigned int years, unsigned int months, unsigned int days, unsigned int hours, unsigned int minutes, unsigned int seconds, unsigned int fraction, int tzHours, int TzMinutes, bool negative = false, int precision = 3);
	SQLDateTime(const std::string & str);
	virtual ~SQLDateTime();

	SQLDateTime & operator = (const SQLDateTime & o);

	bool operator < (const SQLDateTime & o) const;
	bool operator <= (const SQLDateTime & o) const;
	bool operator > (const SQLDateTime & o) const;
	bool operator >= (const SQLDateTime & o) const;
	bool operator == (const SQLDateTime & o) const;
	bool operator != (const SQLDateTime & o) const;

	bool isNull() const;

    //! @deprecated -> has_fraction
	bool has_msecs() const;

    bool has_fraction() const;

	unsigned int years() const;
	unsigned int months() const;
	unsigned int days() const;
	unsigned int hours() const;
	unsigned int minutes() const;
	unsigned int seconds() const;

    unsigned int fraction() const;


    int precision() const;

    unsigned int milliseconds() const;
    unsigned int microseconds() const;
    unsigned int nanoseconds() const;

    bool daylightSaveTime() const;
	bool negative() const;

    std::chrono::system_clock::time_point toTimePoint() const;

	std::string toString() const;

	tm to_tm() const;
	time_t to_time_t() const;

private:
    //! @deprecated -> fraction
    int msecs() const;
    //! @deprecated -> precision
    short ms_precision() const;

};

}

#endif /* INCLUDE_SASSQL_SQLDATETIME_H_ */
