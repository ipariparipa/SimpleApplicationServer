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

#ifndef sasClient__string_h
#define sasClient__string_h

#include "sasbasetypes.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct
	{
		size_t size;
		char * data;
		sas_Bool reference;
	} sas_String;

	extern SAS_CLIENT__FUNCTION sas_String SAS_CLIENT__CALL_CONVENTION sas_String_init(size_t size);
	extern SAS_CLIENT__FUNCTION sas_String SAS_CLIENT__CALL_CONVENTION sas_String_init_ref(void);
	extern SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_String_reset(sas_String * str, size_t size);
	extern SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_String_deinit(sas_String * str);
	extern SAS_CLIENT__FUNCTION sas_String SAS_CLIENT__CALL_CONVENTION sas_String_set(const char * str);
	extern SAS_CLIENT__FUNCTION sas_String SAS_CLIENT__CALL_CONVENTION sas_String_set_ref(const char * str);
	extern SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_String_copy_1(sas_String * dst, const char * src);
	extern SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_String_copy_2(sas_String * dst, const sas_String * src);
	extern SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_String_copy_ref_1(sas_String * dst, const char * src);
	extern SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_String_copy_ref_2(sas_String * dst, const sas_String * src);
	extern SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_String_clear(sas_String * str);

	typedef struct
	{
		size_t size;
		sas_String * data;
	} sas_StringArray;

	extern SAS_CLIENT__FUNCTION sas_StringArray SAS_CLIENT__CALL_CONVENTION sas_StringArray_init(size_t size);
	extern SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_StringArray_reset(sas_StringArray * strarr, size_t size);
	extern SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_StringArray_deinit(sas_StringArray * strarr);

#ifdef __cplusplus
}

#include <string>

namespace SAS {

	namespace Client {

		extern SAS_CLIENT__FUNCTION void string_copy(sas_String & dst, const std::string & src);
		extern SAS_CLIENT__FUNCTION void string_copy_ref(sas_String & dst, const std::string & src);
		extern SAS_CLIENT__FUNCTION sas_String string_set(const std::string & str);
		extern SAS_CLIENT__FUNCTION sas_String string_set_ref(const std::string & str);

	}
}

#endif

#endif // sasClient__string_h
