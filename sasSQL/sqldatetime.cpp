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
#include <cstring>
#include <ctime>

#include <time.h>
#include <chrono>

#if SAS_OS == SAS_OS_WINDOWS
extern "C" char* strptime(const char* s,
	const char* f, struct tm* tm) {
	// Isn't the C++ standard lib nice? std::get_time is defined such that its
	// format parameters are the exact same as strptime. Of course, we have to
	// create a string stream first, and imbue it with the current C locale, and
	// we also have to make sure we return the right things if it fails, or
	// if it succeeds, but this is still far simpler an implementation than any
	// of the versions in any of the C standard libraries.
	std::istringstream input(s);
	input.imbue(std::locale(setlocale(LC_ALL, nullptr)));
	input >> std::get_time(tm, f);
	if (input.fail()) {
		return nullptr;
	}

	auto tmp = (int)input.tellg();
	static const char _end('\0');
	return (char*)(tmp < 0 ? &_end : s + tmp);
}
#endif

namespace SAS {

struct SQLDateTime::Priv
{
	bool isNull = true;

	unsigned int years = 0;
	unsigned int months = 0;
	unsigned int days = 0;
	unsigned int hours = 0;
	unsigned int minutes = 0;
	unsigned int seconds = 0;
	int msecs = 0;
	bool daylightSaveTime = false;
	bool negative = false;
	short ms_precision = 0;

	void set_tm(tm & ret)
	{
		std::memset(&ret, 0, sizeof(tm));
		ret.tm_year = years - 1900;
		ret.tm_mon = months - 1;
		ret.tm_mday = days;
		ret.tm_hour = hours;
		ret.tm_min = minutes;
		ret.tm_sec = seconds;
		ret.tm_isdst = daylightSaveTime;
	}

	static void to_tm(unsigned int years, unsigned int months, unsigned int days, unsigned int hours, unsigned int minutes, unsigned int seconds, tm & ret)
	{
		std::memset(&ret, 0, sizeof(tm));
		ret.tm_year = years - 1900;
		ret.tm_mon = months - 1;
		ret.tm_mday = days;
		ret.tm_hour = hours;
		ret.tm_min = minutes;
		ret.tm_sec = seconds;
	}

	static bool to_tm(const char * str, tm & ret)
	{
		std::memset(&ret, 0, sizeof(tm));
		auto tmp = strptime(str, "%Y-%m-%dT%H:%M:%S", &ret);
		if (!tmp && !*tmp)
			return false;
		ret.tm_isdst = -1;
		std::mktime(&ret);
		return true;
	}

	static std::string to_string(const tm & v)
	{
		std::stringstream ss;
		ss << std::setfill('0') << std::setw(4) << v.tm_year + 1900 << "-";
		ss << std::setfill('0') << std::setw(2) << v.tm_mon + 1 << "-";
		ss << std::setfill('0') << std::setw(2) << v.tm_mday << "T";
		ss << std::setfill('0') << std::setw(2) << v.tm_hour << ":";
		ss << std::setfill('0') << std::setw(2) << v.tm_min << ":";
		ss << std::setfill('0') << std::setw(2) << v.tm_sec;
		return ss.str();
	}

	void from_tm(const tm * t)
	{
		years = t->tm_year + 1900;
		months = t->tm_mon + 1;
		days = t->tm_mday;
		hours = t->tm_hour;
		minutes = t->tm_min;
		seconds = t->tm_sec;
		daylightSaveTime = t->tm_isdst != 0;
	}

	static void to_tm(time_t t, tm & ret)
	{
#if SAS_OS == SAS_OS_LINUX
		gmtime_r(&t, &ret);
#else
		gmtime_s(&ret, &t);
#endif
	}

};


SQLDateTime::SQLDateTime(const SQLDateTime & o) : priv(new Priv(*o.priv))
{ }

SQLDateTime::SQLDateTime() : priv(new Priv)
{
	priv->years = priv->months = priv->days = priv->hours = priv->minutes = priv->seconds = 0;
	priv->msecs = -1;
	priv->negative = priv->daylightSaveTime = false;
	priv->isNull = true;

	priv->ms_precision = 0;
}

SQLDateTime::SQLDateTime(time_t t) : priv(new Priv)
{
	tm tmp;

	Priv::to_tm(t, tmp);
	priv->from_tm(&tmp);

	priv->ms_precision = 0;
}

SQLDateTime::SQLDateTime(const tm * t) : priv(new Priv)
{
	priv->from_tm(t);

	priv->msecs = -1;
	priv->negative = false;
	priv->isNull = false;
	priv->ms_precision = 0;
}

SQLDateTime::SQLDateTime(const tm * t, unsigned int milliseconds, short ms_precision)
{
	priv->from_tm(t);

	priv->negative = false;
	priv->isNull = false;

	priv->msecs = milliseconds;
	priv->ms_precision = ms_precision;

}

SQLDateTime::SQLDateTime(unsigned int years, unsigned int months, unsigned int days, unsigned int hours, unsigned int minutes, unsigned int seconds, int msecs, bool negative, short ms_precision) : priv(new Priv)
{
	priv->years = years;
	priv->months = months;
	priv->days = days;
	priv->hours = hours;
	priv->minutes = minutes;
	priv->seconds = seconds;
	priv->msecs = msecs;
	priv->negative = negative;
	priv->isNull = false;
	priv->ms_precision = ms_precision;
	priv->daylightSaveTime = false;

	tm tmp;
	priv->set_tm(tmp);
	tmp.tm_isdst = -1;
	mktime(&tmp);
	priv->daylightSaveTime = tmp.tm_isdst != 0;
}

SQLDateTime::SQLDateTime(unsigned int years, unsigned int months, unsigned int days, unsigned int hours, unsigned int minutes, unsigned int seconds, int msecs, int tzHours, int TzMinutes, bool negative, short ms_precision)
{
	std::tm tm;
	Priv::to_tm(years, months, days, hours, minutes, seconds, tm);
	auto tp = std::chrono::system_clock::from_time_t(mktime(&tm));
	tp += std::chrono::hours(tzHours);
	tp += std::chrono::minutes(TzMinutes);
	if (msecs >= 0)
		tp += std::chrono::milliseconds(msecs);

	auto t = std::chrono::system_clock::to_time_t(tp);
	Priv::to_tm(t, tm);
	priv->from_tm(&tm);
	
	if (msecs >= 0)
	{
		priv->msecs = (int) (tp - std::chrono::system_clock::from_time_t(t)).count();
	}

	priv->ms_precision = ms_precision;

/*
	std::tm tmp;
	priv->set_tm(tmp);
	tmp.tm_isdst = -1;
	mktime(&tmp);
	priv->daylightSaveTime = tmp.tm_isdst != 0;
*/
}


SQLDateTime::SQLDateTime(const std::string & str) : priv(new Priv)
{
	tm t;
	if(Priv::to_tm(str.c_str(), t))
	{
		priv->from_tm(&t);

		priv->msecs = -1;
		priv->negative = false;
		priv->isNull = false;
		priv->ms_precision = 0;
	}
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

bool SQLDateTime::operator < (const SQLDateTime & o) const
{
	return to_time_t() < o.to_time_t() ||
			((priv->msecs < 0 ? 0 : priv->msecs)  < (o.priv->msecs < 0 ? 0 : o.priv->msecs));
}

bool SQLDateTime::operator <= (const SQLDateTime & o) const
{
	return operator <(o) || operator ==(o);
}

bool SQLDateTime::operator > (const SQLDateTime & o) const
{
	return to_time_t() > o.to_time_t() ||
			((priv->msecs < 0 ? 0 : priv->msecs) > (o.priv->msecs < 0 ? 0 : o.priv->msecs));
}

bool SQLDateTime::operator >= (const SQLDateTime & o) const
{
	return operator >(o) || operator ==(o);
}

bool SQLDateTime::operator == (const SQLDateTime & o) const
{
	return to_time_t() == o.to_time_t() &&
			((priv->msecs < 0 ? 0 : priv->msecs) == (o.priv->msecs < 0 ? 0 : o.priv->msecs));
}

bool SQLDateTime::operator != (const SQLDateTime & o) const
{
	return to_time_t() != o.to_time_t() &&
			((priv->msecs < 0 ? 0 : priv->msecs) != (o.priv->msecs < 0 ? 0 : o.priv->msecs));
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
	priv->set_tm(ret);
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
