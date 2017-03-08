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

#include "include/sasSQL/sqldatetime.h"

#include <sstream>
#include <iomanip>

namespace SAS {

struct SQLDateTime_priv
{
	bool isNull;

	unsigned int years;
	unsigned int months;
	unsigned int days;
	unsigned int hours;
	unsigned int minutes;
	unsigned int seconds;
	int msecs;
	bool daylightSaveTime;
	bool negative;
	short ms_precision;
};


SQLDateTime::SQLDateTime(const SQLDateTime & o) : priv(new SQLDateTime_priv(*o.priv))
{ }

SQLDateTime::SQLDateTime() : priv(new SQLDateTime_priv)
{
	priv->years = priv->months = priv->days = priv->hours = priv->minutes = priv->seconds = 0;
	priv->msecs = -1;
	priv->negative = priv->daylightSaveTime = false;
	priv->isNull = true;

	priv->ms_precision = 0;
}

SQLDateTime::SQLDateTime(time_t t) : priv(new SQLDateTime_priv)
{
	tm tmp;
#if SAS_OS == SAS_OS_LINUX
	gmtime_r(&t, &tmp);
#else
	gmtime_s(&tmp, &t);
#endif

	priv->years = tmp.tm_year + 1900;
	priv->months = tmp.tm_mon + 1;
	priv->days = tmp.tm_mday;
	priv->hours = tmp.tm_hour;
	priv->minutes = tmp.tm_min;
	priv->seconds = tmp.tm_sec;
	priv->msecs = -1;
	priv->daylightSaveTime = tmp.tm_isdst != 0;

	priv->negative = false;
	priv->isNull = false;

	priv->ms_precision = 0;
}

SQLDateTime::SQLDateTime(tm * t) : priv(new SQLDateTime_priv)
{
	priv->years = t->tm_year + 1900;
	priv->months = t->tm_mon + 1;
	priv->days = t->tm_mday;
	priv->hours = t->tm_hour;
	priv->minutes = t->tm_min;
	priv->seconds = t->tm_sec;
	priv->msecs = -1;
	priv->daylightSaveTime = t->tm_isdst != 0;

	priv->negative = false;
	priv->isNull = false;

	priv->ms_precision = 0;
}

SQLDateTime::SQLDateTime(unsigned int years, unsigned int months, unsigned int days, unsigned int hours, unsigned int minutes, unsigned int seconds, int msecs, bool daylightSaveTime, bool negative, short ms_precision) : priv(new SQLDateTime_priv)
{
	priv->years = years;
	priv->months = months;
	priv->days = days;
	priv->hours = hours;
	priv->minutes = minutes;
	priv->seconds = seconds;
	priv->msecs = msecs;
	priv->daylightSaveTime = daylightSaveTime;
	priv->negative = negative;
	priv->isNull = false;
	priv->ms_precision = ms_precision;
}

SQLDateTime::~SQLDateTime()
{
	delete priv;
}

SQLDateTime & SQLDateTime::operator = (const SQLDateTime & o)
{
	*priv = *o.priv;
	return *this;
}

bool SQLDateTime::isNull() const
{
	return priv->isNull;
}

bool SQLDateTime::has_msecs() const
{
	return priv->msecs >= 0;
}

unsigned int SQLDateTime::years() const
{
	return priv->years;
}

unsigned int SQLDateTime::months() const
{
	return priv->months;
}

unsigned int SQLDateTime::days() const
{
	return priv->days;
}

unsigned int SQLDateTime::hours() const
{
	return priv->hours;
}

unsigned int SQLDateTime::minutes() const
{
	return priv->minutes;
}

unsigned int SQLDateTime::seconds() const
{
	return priv->seconds;
}

int SQLDateTime::msecs() const
{
	return priv->msecs < 0 ? 0 : priv->msecs;
}

bool SQLDateTime::daylightSaveTime() const
{
	return priv->daylightSaveTime;
}

bool SQLDateTime::negative() const
{
	return priv->negative;
}

std::string SQLDateTime::toString() const
{
	std::stringstream ss;
	ss << std::setfill('0');
	if(priv->negative)
		ss << "-";
	ss 	<< std::setw(4) << priv->years
		<< '-'
		<< std::setw(2) << priv->months
		<< '-'
		<< std::setw(2) << priv->days
		<< 'T'
		<< std::setw(2) << priv->hours
		<< ':'
		<< std::setw(2) << priv->minutes
		<< ':'
		<< std::setw(2) << priv->seconds;
	if(priv->msecs >=0)
		ss << '.' << std::setw(priv->ms_precision) << priv->msecs;
	return ss.str();
}


tm SQLDateTime::to_tm() const
{
	tm ret;
	ret.tm_year = priv->years - 1900;
	ret.tm_mon = priv->months - 1;
	ret.tm_mday = priv->days;
	ret.tm_hour = priv->hours;
	ret.tm_min = priv->minutes;
	ret.tm_sec = priv->seconds;
	ret.tm_isdst = priv->daylightSaveTime;
	mktime(&ret);
	return ret;
}

time_t SQLDateTime::to_time_t() const
{
	tm ret;
	ret.tm_year = priv->years - 1900;
	ret.tm_mon = priv->months - 1;
	ret.tm_mday = priv->days;
	ret.tm_hour = priv->hours;
	ret.tm_min = priv->minutes;
	ret.tm_sec = priv->seconds;
	ret.tm_isdst = priv->daylightSaveTime;
	return mktime(&ret);
}

}