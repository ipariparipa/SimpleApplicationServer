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

#include "include/sasClient/sasstring.h"

#include <string.h>
#include <assert.h>

char _sas_empty_string[] = { '\0' };

extern "C" SAS_CLIENT__FUNCTION sas_String SAS_CLIENT__CALL_CONVENTION sas_String_init(size_t size)
{
	sas_String ret = {0, NULL, FALSE};
	sas_String_reset(&ret, size);
	return ret;
}

extern "C" SAS_CLIENT__FUNCTION sas_String SAS_CLIENT__CALL_CONVENTION sas_String_init_ref(void)
{
	sas_String ret = { sizeof(_sas_empty_string), _sas_empty_string, TRUE };
	return ret;
}

extern "C" SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_String_reset(sas_String * str, size_t size)
{
	assert(str);
	if (str->data && !str->reference)
		free(str->data);
	str->data = (char *)malloc(str->size = size + 1);
	str->reference = FALSE;
}

extern "C" SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_String_deinit(sas_String * str)
{
	assert(str);
	if (!str->reference && str->data)
		free(str->data);
	str->data = NULL;
	str->size = 0;
}

extern "C" SAS_CLIENT__FUNCTION sas_String SAS_CLIENT__CALL_CONVENTION sas_String_set(const char * str)
{
	assert(str);
	sas_String ret = sas_String_init(strlen(str));
	strncpy(ret.data, str, ret.size-1);
	return ret;
}

extern "C" SAS_CLIENT__FUNCTION sas_String SAS_CLIENT__CALL_CONVENTION sas_String_set_ref(const char * str)
{
	assert(str);
	sas_String ret = sas_String_init_ref();
	ret.data = (char*)str;
	ret.size = strlen(str);
	return ret;
}

extern "C" SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_String_copy_1(sas_String * dst, const char * src)
{
	assert(dst);
	assert(src);
	size_t s = strlen(src) + 1;
	if (dst->size < s)
		sas_String_reset(dst, s);
	strncpy(dst->data, src, dst->size - 1);
}

extern "C" SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_String_copy_2(sas_String * dst, const sas_String * src)
{
	assert(dst);
	assert(src);
	if (dst->size < src->size)
		sas_String_reset(dst, src->size - 1);
	strncpy(dst->data, src->data, dst->size - 1);
}

extern "C" SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_String_copy_ref_1(sas_String * dst, const char * src)
{
	assert(dst);
	assert(src);
	sas_String_deinit(dst);
	dst->reference = TRUE;
	dst->data = (char*)src;
	dst->size = strlen(src) + 1;
}

extern "C" SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_String_copy_ref_2(sas_String * dst, const sas_String * src)
{
	assert(dst);
	assert(src);
	sas_String_deinit(dst);
	dst->reference = TRUE;
	dst->data = src->data;
	dst->size = src->size;
}

extern "C" SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_String_clear(sas_String * str)
{
	assert(str);
	if (str->reference)
	{
		str->data = _sas_empty_string;
		str->size = sizeof(_sas_empty_string);
	}
	else if (str->size)
		*str->data = '\0';
}



extern "C" SAS_CLIENT__FUNCTION sas_StringArray SAS_CLIENT__CALL_CONVENTION sas_StringArray_init(size_t size)
{
	sas_StringArray ret = {0, NULL};
	sas_StringArray_reset(&ret, size);
	return ret;
}

extern "C" SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_StringArray_reset(sas_StringArray * strarr, size_t size)
{
	assert(strarr);
	if (strarr->data)
		free(strarr->data);
	size_t s = (strarr->size = size) * sizeof(sas_String);
	strarr->data = (sas_String*)malloc(s);
	memset(strarr->data, 0, s);
}

extern "C" SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_StringArray_deinit(sas_StringArray * strarr)
{
	assert(strarr);
	if (strarr->data)
		free(strarr->data);
	strarr->data = NULL;
	strarr->size = 0;
}

namespace SAS {

	namespace Client {

		extern SAS_CLIENT__FUNCTION void string_copy(sas_String & dst, const std::string & src)
		{
			sas_String_reset(&dst, src.size());
			strncpy(dst.data, src.c_str(), dst.size);
		}

		extern SAS_CLIENT__FUNCTION void string_copy_ref(sas_String & dst, const std::string & src)
		{
			sas_String_deinit(&dst);
			dst.data = (char*)src.c_str();
			dst.size = src.size();
			dst.reference = TRUE;
		}

		extern SAS_CLIENT__FUNCTION sas_String string_set(const std::string & str)
		{
			auto ret = sas_String_init(str.size());
			strncpy(ret.data, str.c_str(), ret.size);
			return ret;
		}

		extern SAS_CLIENT__FUNCTION sas_String string_set_ref(const std::string & str)
		{
			sas_String ret = { str.size(), (char*)str.c_str(), TRUE };
			return ret;
		}

	}
}
