/*
    This file is part of sasCore.

    sasCore is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasCore is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasCore.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef sasCore__tools_h
#define sasCore__tools_h

#include "config.h"

#if SAS_OS == SAS_OS_LINUX
#elif SAS_OS == SAS_OS_WINDOWS
#include <Windows.h>
#else
#  error to be implemented
#endif

#include <string>
#include <list>
#include <vector>
#include <cctype>

namespace SAS  {

#if SAS_OS == SAS_OS_LINUX
#elif SAS_OS == SAS_OS_WINDOWS
	extern SAS_CORE__FUNCTION std::string win_getLastErrorMessage();
	
	extern SAS_CORE__FUNCTION std::string win_getLastErrorMessage(long & errorCode);

	extern SAS_CORE__FUNCTION std::string win_getErrorMessage(long errorCode);

	extern SAS_CORE__FUNCTION bool win_getEnv(const std::string & name, std::string & ret);
#endif

	extern SAS_CORE__FUNCTION void str_split(const std::string & s, char delim, std::list<std::string> & elems);

	extern SAS_CORE__FUNCTION std::list<std::string> str_split(const std::string & s, char delim);

	template<typename std__container, typename delim_T = typename std__container::value_type::value_type>
	void str_join(const std__container & elems, const delim_T & delim, typename std__container::value_type & s)
	{
		size_t i(0), l(elems.size());
		for (auto & e : elems)
		{
			s += e;
			if (i++ + 1 < l)
				s += delim;
		}
	}

	template<typename std__container, typename delim_T = typename std__container::value_type::value_type>
	typename std__container::value_type str_join(const std__container & elems, const delim_T & delim)
	{
		typename std__container::value_type s;
		str_join(elems, delim, s);
		return s;
	}

	template<typename str_type>
	void str_tolower(const str_type & str, str_type & ret)
	{
		ret = str;
		for (auto & c : ret)
			c = std::tolower(c);
	}

	template<typename str_type>
	str_type str_tolower(const str_type & str)
	{
		str_type ret;
		str_tolower(str, ret);
		return ret;
	}

	template<typename str_type>
	void str_toupper(const str_type & str, str_type & ret)
	{
		ret = str;
		for (auto & c : ret)
			c = std::toupper(c);
	}

	template<typename str_type>
	str_type str_toupper(const str_type & str)
	{
		auto ret;
		str_toupper(str, ret);
		return ret;
	}

	extern SAS_CORE__FUNCTION std::string genRandomString(size_t len);

}

#endif // sasCore__tools_h
