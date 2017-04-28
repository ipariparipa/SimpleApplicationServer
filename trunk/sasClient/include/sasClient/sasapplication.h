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

#ifndef sasClient__application_h
#define sasClient__application_h

#include "config.h"
#include "sasconfigreader.h"
#include "sasstring.h"

#ifdef __cplusplus

#include <sasCore/application.h>

typedef SAS::Application * sas_Application_T;

extern "C" {
#else
typedef struct Application { void * obj; } sas_Application_T;
#endif

	typedef sas_String(SAS_CLIENT__CALL_CONVENTION *sas_Application_version_T)(void);
	typedef sas_ConfigReader_T(SAS_CLIENT__CALL_CONVENTION *sas_Application_configReader_T)(void);

	typedef struct
	{
		sas_Application_version_T version; //=NULL
		sas_Application_configReader_T configReader;
	} sas_Application_functions_T;

	extern SAS_CLIENT__FUNCTION sas_Application_T SAS_CLIENT__CALL_CONVENTION sas_Application_init(int argc, char **argv, const sas_Application_functions_T * functions, sas_ErrorCollector_T ec);
	extern SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_Application_deinit(sas_Application_T obj);

	extern SAS_CLIENT__FUNCTION sas_String SAS_CLIENT__CALL_CONVENTION sas_Application_version(sas_Application_T obj);
	extern SAS_CLIENT__FUNCTION sas_ConfigReader_T SAS_CLIENT__CALL_CONVENTION sas_Application_configReader(sas_Application_T obj);	

#ifdef __cplusplus
}
#endif

#endif // sasClient__application_h
