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

#include "include/sasCore/tools.h"

#include <vector>
#include <sstream>

namespace SAS {

#if SAS_OS == SAS_OS_LINUX
#elif SAS_OS == SAS_OS_WINDOWS
	extern SAS_CORE__FUNCTION std::string win_getLastErrorMessage()
	{
		long ec;
		return win_getLastErrorMessage(ec);
	}

	extern SAS_CORE__FUNCTION std::string win_getLastErrorMessage(long & errorCode)
	{
		//Get the error message, if any.
		errorCode = ::GetLastError();
		if (errorCode == 0)
			return "(no error)";
		return win_getErrorMessage(errorCode);
	}

	extern SAS_CORE__FUNCTION std::string win_getErrorMessage(long errorCode)
	{
		LPSTR messageBuffer = nullptr;
		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

		std::string ret(messageBuffer);
		LocalFree(messageBuffer);

		return ret;
	}

	extern SAS_CORE__FUNCTION bool win_getEnv(const std::string & name, std::string & ret)
	{
		DWORD size;
		if (!(size = GetEnvironmentVariableA(name.c_str(), NULL, 0)))
			return false;
		std::vector<char> tmp(size);
		if (!(size = GetEnvironmentVariableA(name.c_str(), tmp.data(), size)))
			return false;
		ret = tmp.data();
		return true;
	}

#endif

	extern SAS_CORE__FUNCTION void str_split(const std::string & s, char delim, std::list<std::string> & elems)
	{
		std::stringstream ss;
		ss.str(s);
		std::string item;
		while (std::getline(ss, item, delim))
			elems.push_back(item);
	}

	extern SAS_CORE__FUNCTION std::list<std::string> str_split(const std::string & s, char delim)
	{
		std::list<std::string> elems;
		str_split(s, delim, elems);
		return elems;
	}

	extern SAS_CORE__FUNCTION  std::string genRandomString(size_t len)
	{
		static const char alphanum[] =
		"0123456789"
		//"!@#$%^&*"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";

		std::vector<char> str(len);
		for(size_t i(0); i < len; ++i)
			str[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
		std::string ret;
		ret.append(str.data(), len);
		return ret;
	}

}
