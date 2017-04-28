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

#include "include/sasClient/sastcllist.h"
#include <assert.h>

extern "C" SAS_CLIENT__FUNCTION sas_TCLList_T SAS_CLIENT__CALL_CONVENTION sas_TCLList_init(void)
{
	return new SAS::TCLList;
}

extern "C" SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_TCLList_deinit(sas_TCLList_T obj)
{
	delete obj;
}

extern "C" SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_TCLList_isNull(sas_TCLList_T obj, const char * str)
{
	assert(obj);
	return (sas_Bool)obj->isNull();
}

extern "C" SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_TCLList_fromString(sas_TCLList_T obj, const char * str)
{
	assert(obj);
	assert(str);
	return (sas_Bool)obj->fromString(str);
}

extern "C" SAS_CLIENT__FUNCTION size_t SAS_CLIENT__CALL_CONVENTION sas_TCLList_length(sas_TCLList_T obj)
{
	assert(obj);
	return obj->length();
}

extern "C" SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_TCLList_appendString(sas_TCLList_T obj, const sas_TCLList_T lst)
{
	assert(obj);
	assert(lst);
	obj->append(*lst);
}

extern "C" SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_TCLList_appendList(sas_TCLList_T obj, const char * str)
{
	assert(obj);
	assert(str);
	obj->append(str);
}

extern "C" SAS_CLIENT__FUNCTION sas_TCLList_T SAS_CLIENT__CALL_CONVENTION sas_TCLList_getList(sas_TCLList_T obj, size_t idx)
{
	assert(obj);
	auto tmp = new SAS::TCLList;
	if(!obj->getList(idx, *tmp))
	{
		delete tmp;
		return nullptr;
	}
	return tmp;
}

extern "C" SAS_CLIENT__FUNCTION const char * SAS_CLIENT__CALL_CONVENTION sas_TCLList_at(sas_TCLList_T obj, size_t idx)
{
	assert(obj);
	return obj->at(idx).c_str();
}

extern "C" SAS_CLIENT__FUNCTION sas_String SAS_CLIENT__CALL_CONVENTION sas_TCLList_toString(sas_TCLList_T obj)
{
	assert(obj);
	return SAS::Client::string_set(obj->toString());
}
