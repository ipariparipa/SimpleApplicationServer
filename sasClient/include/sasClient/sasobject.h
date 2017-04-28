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

#ifndef sasClient__object_h
#define sasClient__object_h

#include "sasbasetypes.h"
#include "sasstring.h"

#ifdef __cplusplus
#  include <sasCore/object.h>

typedef SAS::Object * sas_Object_T;

extern "C" {
#else
typedef struct { void* obj; } sas_Object_T;
#endif

extern SAS_CLIENT__FUNCTION sas_String SAS_CLIENT__CALL_CONVENTION sas_Object_name(sas_Object_T obj);
extern SAS_CLIENT__FUNCTION sas_String SAS_CLIENT__CALL_CONVENTION sas_Object_type(sas_Object_T obj);

#ifdef __cplusplus
}
#endif

#endif // sasClient__object_h
