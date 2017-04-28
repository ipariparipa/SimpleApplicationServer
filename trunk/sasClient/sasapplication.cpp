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

#include "include/sasClient/sasapplication.h"

#include <assert.h>

class WApplication : public SAS::Application
{
public:
	WApplication(int argc, char **argv, const sas_Application_functions_T & functions) : SAS::Application(argc, argv), _functions(functions)
	{ }

	virtual inline std::string version() const 
	{ 
		if (_functions.version)
		{
			auto tmp = _functions.version();
			std::string ret(tmp.data);
			sas_String_deinit(&tmp);
			return ret;
		}
		return SAS::Application::version();
	}

	virtual SAS::ConfigReader * configReader()
	{
		return _functions.configReader();
	}

private:
	sas_Application_functions_T _functions;
};

extern "C" SAS_CLIENT__FUNCTION sas_Application_T SAS_CLIENT__CALL_CONVENTION sas_Application_init(int argc, char **argv, const sas_Application_functions_T * functions, sas_ErrorCollector_T ec)
{
	assert(functions->configReader);
	auto a = new WApplication(argc, argv, *functions);
	if (!a->init(*ec))
	{
		delete a;
		return nullptr;
	}

	return a;
}

extern "C" SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_Application_deinit(sas_Application_T obj)
{
	assert(obj);
	obj->deinit();
	delete obj;
}

extern "C" SAS_CLIENT__FUNCTION sas_String SAS_CLIENT__CALL_CONVENTION sas_Application_version(sas_Application_T obj)
{
	assert(obj);
	return SAS::Client::string_set(obj->version());
}

extern "C" SAS_CLIENT__FUNCTION sas_ConfigReader_T SAS_CLIENT__CALL_CONVENTION sas_Application_configReader(sas_Application_T obj)
{
	assert(obj);
	return obj->configReader();
}
