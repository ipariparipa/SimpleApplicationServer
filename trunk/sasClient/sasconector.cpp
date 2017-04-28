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

#include "include/sasClient/sasconnector.h"

extern "C" SAS_CLIENT__FUNCTION sas_Bool sas_Connector_connect(sas_Connector_T obj, sas_ErrorCollector_T ec)
{
	return (sas_Bool)obj->connect(*ec);
}

extern "C" SAS_CLIENT__FUNCTION sas_Bool sas_Connector_getModuleInfo(sas_Connector_T obj, const char * moduleName, sas_String * description, sas_String * version, sas_ErrorCollector_T ec)
{
	std::string tmp_description, tmp_version;
	if (!obj->getModuleInfo(moduleName, tmp_description, tmp_version, *ec))
		return FALSE;
	SAS::Client::string_copy(*description, tmp_description);
	SAS::Client::string_copy(*version, tmp_version);
	return TRUE;
}

extern "C" SAS_CLIENT__FUNCTION sas_Connection_T sas_Connector_createConnection(sas_Connector_T obj, const char * moduleName, const char * invokerName, sas_ErrorCollector_T ec)
{
	return obj->createConnection(moduleName, invokerName, *ec);
}

extern "C" SAS_CLIENT__FUNCTION sas_Object_T sas_Connector_asObject(sas_Connector_T obj)
{
	return obj;
}



extern "C" SAS_CLIENT__FUNCTION sas_Bool sas_Connection_getSession(sas_Connection_T obj, sas_ErrorCollector_T ec)
{
	return (sas_Bool)obj->getSession(*ec);
}

extern "C" SAS_CLIENT__FUNCTION sas_Invoker_Status sas_Connection_invoke(sas_Connection_T obj, const sas_BinaryData * input, sas_BinaryData * output, sas_ErrorCollector_T ec)
{
	std::vector<char> tmp_input, tmp_output;

	SAS::Client::binarydata_copy(tmp_input, *input);

	SAS::Invoker::Status ret;
	if ((ret = obj->invoke(tmp_input, tmp_output, *ec)) != SAS::Invoker::Status::OK)
		return (sas_Invoker_Status)ret;
	
	SAS::Client::binarydata_copy(*output, tmp_output);

	return sas_Invoker_Status__OK;
}

