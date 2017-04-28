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

#include "include/sasClient/sastcldatawriter.h"
#include <assert.h>

extern "C" SAS_CLIENT__FUNCTION sas_TCLDataWriter_T SAS_CLIENT__CALL_CONVENTION sas_TCLDataWriter_init(short version)
{
	return new SAS::TCLDataWriter(version);
}

extern "C" SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_TCLDataWriter_deinit(sas_TCLDataWriter_T obj)
{
	delete obj;
}

extern "C" SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_TCLDataWriter_addScript(sas_TCLDataWriter_T obj, const char * script, sas_ErrorCollector_T ec)
{
	assert(obj);
	return (sas_Bool)obj->addScript(script, *ec);
}

extern "C" SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_TCLDataWriter_addBlobSetter(sas_TCLDataWriter_T obj, const char * blob_name, const sas_BinaryData * data, sas_ErrorCollector_T ec)
{
	assert(obj);
	assert(blob_name);
	assert(data);
	return (sas_Bool)obj->addBlobSetter(blob_name, SAS::Client::binarydata_copy(*data), *ec);
}

extern "C" SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_TCLDataWriter_addBlobGetter_1(sas_TCLDataWriter_T obj, const char * blob_name, sas_ErrorCollector_T ec)
{
	assert(obj);
	assert(blob_name);
	return (sas_Bool)obj->addBlobGetter(blob_name, *ec);
}

extern "C" SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_TCLDataWriter_addBlobGetter_2(sas_TCLDataWriter_T obj, sas_ErrorCollector_T ec)
{
	assert(obj);
	return (sas_Bool)obj->addBlobGetter(*ec);
}

extern "C" SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_TCLDataWriter_addBlobRemover_1(sas_TCLDataWriter_T obj, const char * blob_name, sas_ErrorCollector_T ec)
{
	assert(obj);
	assert(blob_name);
	return (sas_Bool)obj->addBlobRemover(blob_name, *ec);
}

extern "C" SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_TCLDataWriter_addBlobRemover_2(sas_TCLDataWriter_T obj, sas_ErrorCollector_T ec)
{
	assert(obj);
	return (sas_Bool)obj->addBlobRemover(*ec);
}

extern "C" SAS_CLIENT__FUNCTION sas_BinaryData SAS_CLIENT__CALL_CONVENTION sas_TCLDataWriter_data(sas_TCLDataWriter_T obj)
{
	assert(obj);
	return SAS::Client::binarydata_set(obj->data());
}

extern "C" SAS_CLIENT__FUNCTION sas_BinaryData SAS_CLIENT__CALL_CONVENTION sas_TCLDataWriter_data_ref(sas_TCLDataWriter_T obj)
{
	assert(obj);
	return SAS::Client::binarydata_set_ref(obj->data());
}
