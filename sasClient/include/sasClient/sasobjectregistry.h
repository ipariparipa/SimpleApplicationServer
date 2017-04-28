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

#ifndef sasClient__sasobjectregistry_h
#define sasClient__sasobjectregistry_h

#include "sasobject.h"
#include "saserrorcollector.h"
#include "sasconnector.h"

#ifdef __cplusplus
#  include <sasCore/objectregistry.h>

typedef SAS::ObjectRegistry * sas_ObjectRegistry_T;

extern "C" {
#else
typedef struct {void * obj; } sas_ObjectRegistry_T;
#endif

extern SAS_CLIENT__FUNCTION sas_Object_T sas_ObjectRegistry_getObject(sas_ObjectRegistry_T obj, const char * type, const char * name, sas_ErrorCollector_T ec);
extern SAS_CLIENT__FUNCTION sas_Connector_T sas_ObjectRegistry_getConnector(sas_ObjectRegistry_T obj, const char * name, sas_ErrorCollector_T ec);

#ifdef __cplusplus
}
#endif


#endif // sasClient__sasobjectregistry_h
