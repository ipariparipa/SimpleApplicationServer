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

#include "include/sasClient/sasbinarydata.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

extern "C" SAS_CLIENT__FUNCTION sas_BinaryData SAS_CLIENT__CALL_CONVENTION sas_BinaryData_init(size_t size)
{
	sas_BinaryData ret = { 0, NULL, FALSE };
	sas_BinaryData_reset(&ret, size);
	return ret;
}

extern "C" SAS_CLIENT__FUNCTION sas_BinaryData SAS_CLIENT__CALL_CONVENTION sas_BinaryData_init_ref(void)
{
	sas_BinaryData ret = { 0, NULL, TRUE };
	return ret;
}

extern "C" SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_BinaryData_reset(sas_BinaryData * data, size_t size)
{
	assert(data);
	if (data->data && !data->reference)
		free(data->data);
	data->data = (char *)malloc(data->size = size);
	data->reference = FALSE;
}

extern "C" SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_BinaryData_deinit(sas_BinaryData * data)
{
	assert(data);
	if (!data->reference && data->data)
		free(data->data);
	data->data = NULL;
	data->size = 0;
}

extern "C" SAS_CLIENT__FUNCTION sas_BinaryData SAS_CLIENT__CALL_CONVENTION sas_BinaryData_set(const char * data, size_t size)
{
	assert(data);
	auto ret = sas_BinaryData_init(size);
	memcpy(ret.data, data, ret.size);
	return ret;
}

extern "C" SAS_CLIENT__FUNCTION sas_BinaryData SAS_CLIENT__CALL_CONVENTION sas_BinaryData_set_ref(const char * data, size_t size)
{
	assert(data);
	auto ret = sas_BinaryData_init_ref();
	ret.data = (char*)data;
	ret.size = size;
	return ret;
}

extern "C" SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_BinaryData_copy_1(sas_BinaryData * dst, const char * src, size_t size)
{
	assert(dst);
	assert(src);
	if (dst->size < size)
		sas_BinaryData_reset(dst, size);
	memcpy(dst->data, src, size);
}

extern "C" SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_BinaryData_copy_2(sas_BinaryData * dst, const sas_BinaryData * src)
{
	assert(dst);
	assert(src);
	if (dst->size < src->size)
		sas_BinaryData_reset(dst, src->size);
	memcpy(dst->data, src->data, src->size);
}

extern "C" SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_BinaryData_copy_ref_1(sas_BinaryData * dst, const char * src, size_t size)
{
	assert(dst);
	assert(src);
	sas_BinaryData_deinit(dst);
	dst->reference = TRUE;
	dst->data = (char*)src;
	dst->size = size;
}

extern "C" SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_BinaryData_copy_ref_2(sas_BinaryData * dst, const sas_BinaryData * src)
{
	assert(dst);
	assert(src);
	sas_BinaryData_deinit(dst);
	dst->reference = TRUE;
	dst->data = src->data;
	dst->size = src->size;
}

namespace SAS {

	namespace Client {

		extern SAS_CLIENT__FUNCTION void binarydata_copy(sas_BinaryData & dst, const std::vector<char> & src)
		{
			sas_BinaryData_reset(&dst, src.size());
			memcpy(dst.data, src.data(), dst.size);
		}

		extern SAS_CLIENT__FUNCTION void binarydata_copy(std::vector<char> & dst, const sas_BinaryData & src)
		{
			dst.resize(src.size);
			memcpy(dst.data(), src.data, dst.size());
		}

		extern SAS_CLIENT__FUNCTION void binarydata_copy_ref(sas_BinaryData & dst, const std::vector<char> & src)
		{
			sas_BinaryData_deinit(&dst);
			dst.data = (char*)src.data();
			dst.size = src.size();
			dst.reference = TRUE;
		}

		extern SAS_CLIENT__FUNCTION sas_BinaryData binarydata_set(const std::vector<char> & data)
		{
			auto ret = sas_BinaryData_init(data.size());
			memcpy(ret.data, data.data(), ret.size);
			return ret;
		}

		extern SAS_CLIENT__FUNCTION sas_BinaryData binarydata_set_ref(const std::vector<char> & data)
		{
			sas_BinaryData ret = { data.size(), (char*)data.data(), TRUE };
			return ret;
		}

	}
}
