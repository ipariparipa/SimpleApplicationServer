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

#ifndef sasClient__sasserver_h
#define sasClient__sasserver_h

#include "sasapplication.h"

#include "config.h"
#include "sasconfigreader.h"
#include "sasstring.h"

#ifdef __cplusplus

#include <sasBasics/server.h>

typedef SAS::Server * sas_Server_T;

extern "C" {
#else
typedef struct Server { void * obj; } sas_Server_T;
#endif

typedef struct
{
	sas_Application_functions_T application;
} sas_Server_functions_T;

extern SAS_CLIENT__FUNCTION sas_Server_T SAS_CLIENT__CALL_CONVENTION sas_Server_init(int argc, char **argv, const sas_Server_functions_T * functions, sas_ErrorCollector_T ec);
extern SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_Server_deinit(sas_Server_T obj);

extern SAS_CLIENT__FUNCTION sas_Application_T SAS_CLIENT__CALL_CONVENTION sas_Server_asApplication(sas_Server_T obj);

extern SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_Server_run(sas_Server_T obj);

#ifdef __cplusplus
}
#endif

#endif // sasClient__sasserver_h
