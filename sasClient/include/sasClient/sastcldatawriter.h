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

#ifndef INCLUDE_SASCLIENT_SASTCLDATAWRITER_H_
#define INCLUDE_SASCLIENT_SASTCLDATAWRITER_H_

#include "sasbasetypes.h"
#include "saserrorcollector.h"
#include "sasbinarydata.h"

#ifdef __cplusplus
#  include <sasTCLTools/tcldatawriter.h>

typedef SAS::TCLDataWriter * sas_TCLDataWriter_T;

extern "C" {
#else
typedef struct { void * obj; } sas_TCLDataWriter_T;
#endif

	extern SAS_CLIENT__FUNCTION sas_TCLDataWriter_T SAS_CLIENT__CALL_CONVENTION sas_TCLDataWriter_init(short version);
	extern SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_TCLDataWriter_deinit(sas_TCLDataWriter_T obj);

	extern SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_TCLDataWriter_addScript(sas_TCLDataWriter_T obj, const char * script, sas_ErrorCollector_T ec);
	extern SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_TCLDataWriter_addBlobSetter(sas_TCLDataWriter_T obj, const char * blob_name, const sas_BinaryData * data, sas_ErrorCollector_T ec);
	extern SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_TCLDataWriter_addBlobGetter_1(sas_TCLDataWriter_T obj, const char * blob_name, sas_ErrorCollector_T ec);
	extern SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_TCLDataWriter_addBlobGetter_2(sas_TCLDataWriter_T obj, sas_ErrorCollector_T ec);
	extern SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_TCLDataWriter_addBlobRemover_1(sas_TCLDataWriter_T obj, const char * blob_name, sas_ErrorCollector_T ec);
	extern SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_TCLDataWriter_addBlobRemover_2(sas_TCLDataWriter_T obj, sas_ErrorCollector_T ec);

	extern SAS_CLIENT__FUNCTION sas_BinaryData SAS_CLIENT__CALL_CONVENTION sas_TCLDataWriter_data(sas_TCLDataWriter_T obj);
	extern SAS_CLIENT__FUNCTION sas_BinaryData SAS_CLIENT__CALL_CONVENTION sas_TCLDataWriter_data_ref(sas_TCLDataWriter_T obj);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_SASCLIENT_SASTCLDATAWRITER_H_ */
