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

#include "include/sasClient/sasobjectregistry.h"

extern "C" SAS_CLIENT__FUNCTION sas_Object_T sas_ObjectRegistry_getObject(sas_ObjectRegistry_T obj, const char * type, const char * name, sas_ErrorCollector_T ec)
{
	return obj->getObject<SAS::Object>(type, name, *ec);
}

extern "C" SAS_CLIENT__FUNCTION sas_Connector_T sas_ObjectRegistry_getConnector(sas_ObjectRegistry_T obj, const char * name, sas_ErrorCollector_T ec)
{
	return obj->getObject<SAS::Connector>(SAS_OBJECT_TYPE__CONNECTOR, name, *ec);
}
