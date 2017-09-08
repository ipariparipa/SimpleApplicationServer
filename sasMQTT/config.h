/*
This file is part of sasMQTT.

sasMQTT is free software: you can redistribute it and/or modify
it under the terms of the Lesser GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

sasMQTT is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with sasMQTT.  If not, see <http://www.gnu.org/licenses/>
*/

#ifndef sasMQTT__config_h
#define sasMQTT__config_h

#include <sasCore/config.h>

#if SAS_OS == SAS_OS_LINUX 
#  define SAS_MQTT__CLASS 
#  define SAS_MQTT__FUNCTION 
#elif SAS_OS == SAS_OS_WINDOWS 
#  ifdef SAS_MQTT__IMPL
#    define SAS_MQTT__CLASS __declspec(dllexport)
#    define SAS_MQTT__FUNCTION __declspec(dllexport)
#  else
#    define SAS_MQTT__CLASS __declspec(dllimport)
#    define SAS_MQTT__FUNCTION __declspec(dllimport)
#  endif
#endif


#endif // sasMQTT__config_h
