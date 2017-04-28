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

#include "include/sasClient/saserrorcollector.h"
#include <assert.h>

class WErrorCollector : public SAS::ErrorCollector
{
public:
	WErrorCollector(sas_ErrorCollector_append_T append_function) : _append_function(append_function), SAS::ErrorCollector()
	{ }

protected:
	virtual void append(long errorCode, const std::string & errorText) final
	{
		assert(_append_function);
		_append_function(errorCode, errorText.c_str());
	}

private:
	sas_ErrorCollector_append_T _append_function;
};

extern "C" SAS_CLIENT__FUNCTION sas_ErrorCollector_T SAS_CLIENT__CALL_CONVENTION sas_ErrorCollector_init(sas_ErrorCollector_append_T append_function)
{
	assert(append_function);
	return new WErrorCollector(append_function);
}

extern "C" SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_ErrorCollector_deinit(sas_ErrorCollector_T obj)
{
	assert(obj);
	delete obj;
}

extern "C" SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_ErrorCollector_add(sas_ErrorCollector_T obj, long errorCode, const char * errorText, sas_String * ret /*=NULL*/)
{
	auto tmp = obj->add(errorCode, errorText);
	if (ret)
		sas_String_copy_1(ret, tmp.c_str());
}
