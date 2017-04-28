/*
This file is part of sasClient.

sasClient is free software: you can redistribute it and/or modify
it under the terms of the Lesser GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

sasClient is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with sasClient.  If not, see <http://www.gnu.org/licenses/>
*/


#ifndef sasClient__basetypes_h
#define sasClient__basetypes_h

#include "config.h"

typedef int sas_Bool;

#ifndef TRUE
#  define TRUE 1
#elif TRUE != 1
#  error incompatible value for TRUE
#endif

#ifndef FALSE
#  define FALSE 0
#elif FALSE != 0
#  error incompatible value for FALSE
#endif

#ifndef NULL
#  define NULL 0
#elif NULL != 0
#  error incompatible value for NULL
#endif

#endif // sasClient__basetypes_h
