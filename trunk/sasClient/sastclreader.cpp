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

#include "include/sasClient/sastcldatareader.h"
#include <assert.h>

extern "C" SAS_CLIENT__FUNCTION sas_TCLDataReader_T SAS_CLIENT__CALL_CONVENTION sas_TCLDataReader_init(void)
{
	return new SAS::TCLDataReader;
}

extern "C" SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_TCLDataReader_deinit(sas_TCLDataReader_T obj)
{
	assert(obj);
	delete obj;
}

extern "C" SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_TCLDataReader_read(sas_TCLDataReader_T obj, const sas_BinaryData * data, sas_ErrorCollector_T ec)
{
	assert(obj);
	assert(data);
	assert(ec);
	return (sas_Bool)obj->read(SAS::Client::binarydata_copy(*data), *ec);
}

extern "C" SAS_CLIENT__FUNCTION short SAS_CLIENT__CALL_CONVENTION sas_TCLDataReader_version(sas_TCLDataReader_T obj)
{
	assert(obj);
	return obj->version();
}

extern "C" SAS_CLIENT__FUNCTION sas_StringArray SAS_CLIENT__CALL_CONVENTION sas_TCLDataReader_tclResults(sas_TCLDataReader_T obj)
{
	assert(obj);
	auto & tmp = obj->tclResults();
	auto ret = sas_StringArray_init(tmp.size());
	size_t i(0);
	for(auto & s : tmp)
		SAS::Client::string_copy(ret.data[i++], s);
	return ret;
}

extern "C" SAS_CLIENT__FUNCTION sas_StringArray SAS_CLIENT__CALL_CONVENTION sas_TCLDataReader_tclResults_ref(sas_TCLDataReader_T obj)
{
	assert(obj);
	auto & tmp = obj->tclResults();
	auto ret = sas_StringArray_init(tmp.size());
	size_t i(0);
	for(auto & s : tmp)
		SAS::Client::string_copy_ref(ret.data[i++], s);
	return ret;
}

extern "C" SAS_CLIENT__FUNCTION sas_StringArray SAS_CLIENT__CALL_CONVENTION sas_TCLDataReader_blobNames(sas_TCLDataReader_T obj)
{
	assert(obj);
	auto & tmp = obj->blobs();
	auto ret = sas_StringArray_init(tmp.size());
	size_t i(0);
	for(auto & t : tmp)
		SAS::Client::string_copy(ret.data[i++], t.first);
	return ret;
}

extern "C" SAS_CLIENT__FUNCTION sas_StringArray SAS_CLIENT__CALL_CONVENTION sas_TCLDataReader_blobNames_ref(sas_TCLDataReader_T obj)
{
	assert(obj);
	auto & tmp = obj->blobs();
	auto ret = sas_StringArray_init(tmp.size());
	size_t i(0);
	for(auto & t : tmp)
		SAS::Client::string_copy_ref(ret.data[i++], t.first);
	return ret;
}

extern "C" SAS_CLIENT__FUNCTION sas_BinaryData SAS_CLIENT__CALL_CONVENTION sas_TCLDataReader_blob(sas_TCLDataReader_T obj, const char * blobName)
{
	assert(obj);
	assert(blobName);
	if(!obj->blobs().count(blobName))
	{
		static const sas_BinaryData tmp = { 0, NULL, FALSE };
		return tmp;
	}
	return SAS::Client::binarydata_set(obj->blobs().at(blobName));
}

extern "C" SAS_CLIENT__FUNCTION sas_BinaryData SAS_CLIENT__CALL_CONVENTION sas_TCLDataReader_blob_ref(sas_TCLDataReader_T obj, const char * blobName)
{
	assert(obj);
	assert(blobName);
	if(!obj->blobs().count(blobName))
	{
		static const sas_BinaryData tmp = { 0, NULL, FALSE };
		return tmp;
	}
	return SAS::Client::binarydata_set_ref(obj->blobs().at(blobName));
}
