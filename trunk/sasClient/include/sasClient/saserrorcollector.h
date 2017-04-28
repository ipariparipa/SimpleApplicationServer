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

#ifndef sasClient__errorcollector_h
#define sasClient__errorcollector_h

#include "sasbasetypes.h"
#include "sasstring.h"

#ifdef __cplusplus

#include <sasCore/errorcollector.h>

typedef SAS::ErrorCollector * sas_ErrorCollector_T;

extern "C" {
#else
typedef struct { void * obj; } sas_ErrorCollector_T;
#endif

	typedef void (SAS_CLIENT__CALL_CONVENTION *sas_ErrorCollector_append_T)(long errorCode, const char * errorText);

	extern SAS_CLIENT__FUNCTION sas_ErrorCollector_T SAS_CLIENT__CALL_CONVENTION sas_ErrorCollector_init(sas_ErrorCollector_append_T append_function);
	extern SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_ErrorCollector_deinit(sas_ErrorCollector_T obj);

	extern SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_ErrorCollector_add(sas_ErrorCollector_T obj, long errorCode, const char * errorText, sas_String * ret /*=NULL*/);

#ifdef __cplusplus
}
#endif

#endif // sasClient__errorcollector_h
