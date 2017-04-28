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

#ifndef sasClient__sasbinarydata_h
#define sasClient__sasbinarydata_h

#include "sasbasetypes.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct
	{
		size_t size;
		char * data;
		sas_Bool reference;
	} sas_BinaryData;

	extern SAS_CLIENT__FUNCTION sas_BinaryData SAS_CLIENT__CALL_CONVENTION sas_BinaryData_init(size_t size);
	extern SAS_CLIENT__FUNCTION sas_BinaryData SAS_CLIENT__CALL_CONVENTION sas_BinaryData_init_ref(void);
	extern SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_BinaryData_reset(sas_BinaryData * data, size_t size);
	extern SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_BinaryData_deinit(sas_BinaryData * data);
	extern SAS_CLIENT__FUNCTION sas_BinaryData SAS_CLIENT__CALL_CONVENTION sas_BinaryData_set(const char * data, size_t size);
	extern SAS_CLIENT__FUNCTION sas_BinaryData SAS_CLIENT__CALL_CONVENTION sas_BinaryData_set_ref(const char * str, size_t size);
	extern SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_BinaryData_copy_1(sas_BinaryData * dst, const char * src, size_t size);
	extern SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_BinaryData_copy_2(sas_BinaryData * dst, const sas_BinaryData * src);
	extern SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_BinaryData_copy_ref_1(sas_BinaryData * dst, const char * src, size_t size);
	extern SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_BinaryData_copy_ref_2(sas_BinaryData * dst, const sas_BinaryData * src);
	extern SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_BinaryData_clear(sas_BinaryData * str);

#ifdef __cplusplus
}

#include <vector>

namespace SAS {

	namespace Client {

		extern SAS_CLIENT__FUNCTION void binarydata_copy(sas_BinaryData & dst, const std::vector<char> & src);
		extern SAS_CLIENT__FUNCTION void binarydata_copy(std::vector<char> & dst, const sas_BinaryData & src);
		extern SAS_CLIENT__FUNCTION void binarydata_copy_ref(sas_BinaryData & dst, const std::vector<char> & src);
		extern SAS_CLIENT__FUNCTION sas_BinaryData binarydata_set(const std::vector<char> & str);
		extern SAS_CLIENT__FUNCTION sas_BinaryData binarydata_set_ref(const std::vector<char> & str);

	}
}

#endif

#endif // sasClient__sasbinarydata_h
