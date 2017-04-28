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

#ifndef INCLUDE_SASCLIENT_SASTCLDATAREADER_H_
#define INCLUDE_SASCLIENT_SASTCLDATAREADER_H_

#include "sasbinarydata.h"
#include "sasstring.h"
#include "saserrorcollector.h"

#ifdef __cplusplus
#  include <sasTCLTools/tcldatareader.h>

typedef SAS::TCLDataReader * sas_TCLDataReader_T;

extern "C" {
#else
typedef struct { void * obj; } sas_TCLDataReader_T;
#endif

	extern SAS_CLIENT__FUNCTION sas_TCLDataReader_T SAS_CLIENT__CALL_CONVENTION sas_TCLDataReader_init(void);
	extern SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_TCLDataReader_deinit(sas_TCLDataReader_T obj);

	extern SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_TCLDataReader_read(sas_TCLDataReader_T obj, const sas_BinaryData * data, sas_ErrorCollector_T ec);

	extern SAS_CLIENT__FUNCTION short SAS_CLIENT__CALL_CONVENTION sas_TCLDataReader_version(sas_TCLDataReader_T obj);
	extern SAS_CLIENT__FUNCTION sas_StringArray SAS_CLIENT__CALL_CONVENTION sas_TCLDataReader_tclResults(sas_TCLDataReader_T obj);
	extern SAS_CLIENT__FUNCTION sas_StringArray SAS_CLIENT__CALL_CONVENTION sas_TCLDataReader_tclResults_ref(sas_TCLDataReader_T obj);
	extern SAS_CLIENT__FUNCTION sas_StringArray SAS_CLIENT__CALL_CONVENTION sas_TCLDataReader_blobNames(sas_TCLDataReader_T obj);
	extern SAS_CLIENT__FUNCTION sas_StringArray SAS_CLIENT__CALL_CONVENTION sas_TCLDataReader_blobNames_ref(sas_TCLDataReader_T obj);
	extern SAS_CLIENT__FUNCTION sas_BinaryData SAS_CLIENT__CALL_CONVENTION sas_TCLDataReader_blob(sas_TCLDataReader_T obj, const char * blobName);
	extern SAS_CLIENT__FUNCTION sas_BinaryData SAS_CLIENT__CALL_CONVENTION sas_TCLDataReader_blob_ref(sas_TCLDataReader_T obj, const char * blobName);

}


#endif /* INCLUDE_SASCLIENT_SASTCLDATAREADER_H_ */
