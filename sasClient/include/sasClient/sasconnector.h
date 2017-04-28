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

#ifndef sasClient__sasconnector_h
#define sasClient__sasconnector_h

#include "saserrorcollector.h"
#include "sasbinarydata.h"
#include "sasobject.h"

#ifdef __cplusplus
#  include <sasCore/connector.h>

typedef SAS::Connector * sas_Connector_T;
typedef SAS::Connection * sas_Connection_T;

extern "C" {
#else
typedef struct { void * obj; } sas_Connector_T;
typedef struct { void * obj; } sas_Connection_T;
#endif

extern SAS_CLIENT__FUNCTION sas_Bool sas_Connector_connect(sas_Connector_T obj, sas_ErrorCollector_T ec);
extern SAS_CLIENT__FUNCTION sas_Bool sas_Connector_getModuleInfo(sas_Connector_T obj, const char * moduleName, sas_String * description, sas_String * version, sas_ErrorCollector_T ec);
extern SAS_CLIENT__FUNCTION sas_Connection_T sas_Connector_createConnection(sas_Connector_T obj, const char * moduleName, const char * invokerName, sas_ErrorCollector_T ec);
extern SAS_CLIENT__FUNCTION sas_Object_T sas_Connector_asObject(sas_Connector_T obj);

typedef enum
{
	sas_Invoker_Status__OK, 
	sas_Invoker_Status__Error, 
	sas_Invoker_Status__FatalError, 
	sas_Invoker_Status__NotImplemented
} sas_Invoker_Status;

extern SAS_CLIENT__FUNCTION sas_Bool sas_Connection_getSession(sas_Connection_T obj, sas_ErrorCollector_T ec);
extern SAS_CLIENT__FUNCTION sas_Invoker_Status sas_Connection_invoke(sas_Connection_T obj, const sas_BinaryData * input, sas_BinaryData * output, sas_ErrorCollector_T ec);

#ifdef __cplusplus
}
#endif


#endif // sasClient__sasobjectregistry_h
