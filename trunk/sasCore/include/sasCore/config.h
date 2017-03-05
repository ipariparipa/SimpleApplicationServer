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

#ifndef INCLUDE_SASCORE_CONFIG_H_
#define INCLUDE_SASCORE_CONFIG_H_

#define SAS_OS_LINUX 1
#define SAS_OS_WINDOWS 2

#ifdef _WINDOWS
#  define SAS_OS SAS_OS_WINDOWS
#else
#  define SAS_OS SAS_OS_LINUX
#endif

#if SAS_OS == SAS_OS_LINUX 
#  define SAS_CORE__CLASS 
#  define SAS_CORE__FUNCTION 
#elif SAS_OS == SAS_OS_WINDOWS 
#  ifdef SAS_CORE__IMPL
#    define SAS_CORE__CLASS __declspec(dllexport)
#    define SAS_CORE__FUNCTION __declspec(dllexport)
#  else
#    define SAS_CORE__CLASS __declspec(dllimport)
#    define SAS_CORE__FUNCTION __declspec(dllimport)
#  endif
#endif

#define SAS_SESSION_CLEANER_INTERVAL 5000
#define SAS_SESSION_MAX_COUNT 2000

//#define SAS_LOG4CXX_ENABLED

#endif /* INCLUDE_SASCORE_CONFIG_H_ */
