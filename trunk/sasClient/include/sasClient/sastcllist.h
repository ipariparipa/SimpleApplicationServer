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

#ifndef INCLUDE_SASCLIENT_SASTCLLIST_H_
#define INCLUDE_SASCLIENT_SASTCLLIST_H_

#include "sasstring.h"

#ifdef __cplusplus
#  include <sasTCLTools/tcllist.h>

typedef SAS::TCLList * sas_TCLList_T;
extern "C" {
#else
typedef struct { void * obj } sas_TCLList_T;
#endif

	extern SAS_CLIENT__FUNCTION sas_TCLList_T SAS_CLIENT__CALL_CONVENTION sas_TCLList_init(void);
	extern SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_TCLList_deinit(sas_TCLList_T obj);

	extern SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_TCLList_isNull(sas_TCLList_T obj, const char * str);
	extern SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_TCLList_fromString(sas_TCLList_T obj, const char * str);

	extern SAS_CLIENT__FUNCTION size_t SAS_CLIENT__CALL_CONVENTION sas_TCLList_length(sas_TCLList_T obj);

	extern SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_TCLList_appendString(sas_TCLList_T obj, const sas_TCLList_T lst);
	extern SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_TCLList_appendList(sas_TCLList_T obj, const char * str);

	extern SAS_CLIENT__FUNCTION sas_TCLList_T SAS_CLIENT__CALL_CONVENTION sas_TCLList_getList(sas_TCLList_T obj, size_t idx);
	extern SAS_CLIENT__FUNCTION const char * SAS_CLIENT__CALL_CONVENTION sas_TCLList_at(sas_TCLList_T obj, size_t idx);

	extern SAS_CLIENT__FUNCTION sas_String SAS_CLIENT__CALL_CONVENTION sas_TCLList_toString(sas_TCLList_T obj);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_SASCLIENT_SASTCLLIST_H_ */
