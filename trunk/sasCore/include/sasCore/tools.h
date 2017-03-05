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

namespace SAS  {

#if SAS_OS == SAS_OS_LINUX
#elif SAS_OS == SAS_OS_WINDOWS
	extern SAS_CORE__FUNCTION std::string win_getLastErrorMessage();
	
	extern SAS_CORE__FUNCTION std::string win_getLastErrorMessage(long & errorCode);

	extern SAS_CORE__FUNCTION std::string win_getErrorMessage(long errorCode);

	extern SAS_CORE__FUNCTION bool win_getEnv(const std::string & name, std::string & ret);
#endif

}

#endif // sasCore__tools_h
