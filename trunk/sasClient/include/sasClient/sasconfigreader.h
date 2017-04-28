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

#ifndef sasClient__configreader_h
#define sasClient__configreader_h

#include "sasbasetypes.h"
#include "saserrorcollector.h"

#ifdef __cplusplus

#include <sasCore/configreader.h>

typedef SAS::ConfigReader * sas_ConfigReader_T;

extern "C" {
#else
typedef struct { void * obj; } sas_ConfigReader_T;
#endif

	typedef sas_Bool(SAS_CLIENT__CALL_CONVENTION *sas_ConfigReader_getEntryAsString_1_T)(const char * path, sas_String * ret, sas_ErrorCollector_T ec);
	typedef sas_Bool(SAS_CLIENT__CALL_CONVENTION *sas_ConfigReader_getEntryAsString_2_T)(const char * path, sas_String * ret, const char * defaultValue, sas_ErrorCollector_T ec);

	typedef sas_Bool(SAS_CLIENT__CALL_CONVENTION *sas_ConfigReader_getEntryAsStringList_1_T)(const char * path, sas_StringArray * ret, sas_ErrorCollector_T ec);
	typedef sas_Bool(SAS_CLIENT__CALL_CONVENTION *sas_ConfigReader_getEntryAsStringList_2_T)(const char * path, sas_StringArray * ret, const sas_StringArray * defaultValue, sas_ErrorCollector_T ec);

	typedef sas_Bool(SAS_CLIENT__CALL_CONVENTION *sas_ConfigReader_getStringEntry_1_T)(const char * path, sas_String * ret, sas_ErrorCollector_T ec);
	typedef sas_Bool(SAS_CLIENT__CALL_CONVENTION *sas_ConfigReader_getStringEntry_2_T)(const char * path, sas_String * ret, const char * defaultValue, sas_ErrorCollector_T ec);

	typedef sas_Bool(SAS_CLIENT__CALL_CONVENTION *sas_ConfigReader_getStringListEntry_1_T)(const char * path, sas_StringArray * ret, sas_ErrorCollector_T ec);
	typedef sas_Bool(SAS_CLIENT__CALL_CONVENTION *sas_ConfigReader_getStringListEntry_2_T)(const char * path, sas_StringArray * ret, const sas_StringArray * defaultValue, sas_ErrorCollector_T ec);

	typedef sas_Bool(SAS_CLIENT__CALL_CONVENTION *sas_ConfigReader_getNumberEntry_1_T)(const char * path, long long * ret, sas_ErrorCollector_T ec);
	typedef sas_Bool(SAS_CLIENT__CALL_CONVENTION *sas_ConfigReader_getNumberEntry_2_T)(const char * path, long long * ret, long long defaultValue, sas_ErrorCollector_T ec);

	typedef sas_Bool(SAS_CLIENT__CALL_CONVENTION *sas_ConfigReader_getBoolEntry_1_T)(const char * path, sas_Bool * ret, sas_ErrorCollector_T ec);
	typedef sas_Bool(SAS_CLIENT__CALL_CONVENTION *sas_ConfigReader_getBoolEntry_2_T)(const char * path, sas_Bool * ret, sas_Bool defaultValue, sas_ErrorCollector_T ec);

	typedef struct
	{
		sas_ConfigReader_getEntryAsStringList_1_T getEntryAsStringList_1;
		sas_ConfigReader_getEntryAsStringList_2_T getEntryAsStringList_2;
		sas_ConfigReader_getEntryAsString_1_T getEntryAsString_1; //=NULL
		sas_ConfigReader_getEntryAsString_2_T getEntryAsString_2; //=NULL
		sas_ConfigReader_getStringEntry_1_T getStringEntry_1; //=NULL
		sas_ConfigReader_getStringEntry_2_T getStringEntry_2; //=NULL
		sas_ConfigReader_getStringListEntry_1_T getStringListEntry_1; //=NULL
		sas_ConfigReader_getStringListEntry_2_T getStringListEntry_2; //=NULL
		sas_ConfigReader_getNumberEntry_1_T getNumberEntry_1; //=NULL
		sas_ConfigReader_getNumberEntry_2_T getNumberEntry_2; //=NULL
		sas_ConfigReader_getBoolEntry_1_T getBoolEntry_1; //=NULL
		sas_ConfigReader_getBoolEntry_2_T getBoolEntry_2; //=NULL
	} sas_ConfigReader_Functions;

	extern SAS_CLIENT__FUNCTION sas_ConfigReader_T SAS_CLIENT__CALL_CONVENTION sas_ConfigReader_init(const sas_ConfigReader_Functions * functions);
	extern SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_ConfigReader_deinit(sas_ConfigReader_T obj);

	extern SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_ConfigReader_getEntryAsString_1(sas_ConfigReader_T obj, const char * path, sas_String * ret, sas_ErrorCollector_T ec);
	extern SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_ConfigReader_getEntryAsString_2(sas_ConfigReader_T obj, const char * path, sas_String * ret, const char * defaultValue, sas_ErrorCollector_T ec);
	extern SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_ConfigReader_getEntryAsStringList_1(sas_ConfigReader_T obj, const char * path, sas_StringArray * ret, sas_ErrorCollector_T ec);
	extern SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_ConfigReader_getEntryAsStringList_2(sas_ConfigReader_T obj, const char * path, sas_StringArray * ret, const sas_StringArray * defaultValue, sas_ErrorCollector_T ec);
	extern SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_ConfigReader_getStringEntry_1(sas_ConfigReader_T obj, const char * path, sas_String * ret, sas_ErrorCollector_T ec);
	extern SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_ConfigReader_getStringEntry_2(sas_ConfigReader_T obj, const char * path, sas_String * ret, const char * defaultValue, sas_ErrorCollector_T ec);
	extern SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_ConfigReader_getStringListEntry_1(sas_ConfigReader_T obj, const char * path, sas_StringArray * ret, sas_ErrorCollector_T ec);
	extern SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_ConfigReader_getStringListEntry_2(sas_ConfigReader_T obj, const char * path, sas_StringArray * ret, const sas_StringArray * defaultValue, sas_ErrorCollector_T ec);
	extern SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_ConfigReader_getNumberEntry_1(sas_ConfigReader_T obj, const char * path, long long * ret, sas_ErrorCollector_T ec);
	extern SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_ConfigReader_getNumberEntry_2(sas_ConfigReader_T obj, const char * path, long long * ret, long long defaultValue, sas_ErrorCollector_T ec);
	extern SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_ConfigReader_getBoolEntry_1(sas_ConfigReader_T obj, const char * path, sas_Bool * ret, sas_ErrorCollector_T ec);
	extern SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_ConfigReader_getBoolEntry_2(sas_ConfigReader_T obj, const char * path, sas_Bool * ret, sas_Bool defaultValue, sas_ErrorCollector_T ec);

#ifdef __cplusplus
}

#endif

#endif // sasClient__configreader_h
