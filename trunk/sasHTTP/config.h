/*
This file is part of sasHTTP.

sasHTTP is free software: you can redistribute it and/or modify
it under the terms of the Lesser GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

sasHTTP is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with sasHTTP.  If not, see <http://www.gnu.org/licenses/>
*/

#ifndef sasHTTP__config_h
#define sasHTTP__config_h

#include <sasCore/config.h>

#if SAS_OS == SAS_OS_LINUX 
#  define SAS_HTTP__CLASS
#  define SAS_HTTP__FUNCTION
#  define SAS_HTTP__HAVE_MICROHTTPD
#elif SAS_OS == SAS_OS_WINDOWS 
#  ifdef SAS_HTTP__IMPL
#    define SAS_HTTP__CLASS __declspec(dllexport)
#    define SAS_HTTP__FUNCTION __declspec(dllexport)
#  else
#    define SAS_HTTP__CLASS __declspec(dllimport)
#    define SAS_HTTP__FUNCTION __declspec(dllimport)
#  endif
//#  define SAS_HTTP__HAVE_MICROHTTPD
#endif

#endif // sasHTTP__config_h
