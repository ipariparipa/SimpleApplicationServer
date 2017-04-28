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

#include "include/sasClient/sasserver.h"

#include <assert.h>

class WServer : public SAS::Server
{
public:
	WServer(int argc, char **argv, sas_Server_functions_T functions) : SAS::Server(argc, argv), _functions(functions)
	{ }

	virtual inline std::string version() const
	{
		if (_functions.application.version)
		{
			auto tmp = _functions.application.version();
			std::string ret(tmp.data);
			sas_String_deinit(&tmp);
			return ret;
		}
		return SAS::Application::version();
	}

	virtual SAS::ConfigReader * configReader()
	{
		return _functions.application.configReader();
	}

private:
	sas_Server_functions_T _functions;
};


extern SAS_CLIENT__FUNCTION sas_Server_T SAS_CLIENT__CALL_CONVENTION sas_Server_init(int argc, char **argv, const sas_Server_functions_T * functions, sas_ErrorCollector_T ec)
{
	assert(functions->application.configReader);
	auto a = new WServer(argc, argv, *functions);
	if (!a->init(*ec))
	{
		delete a;
		return nullptr;
	}

	return a;
}

extern SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_Server_deinit(sas_Server_T obj)
{
	assert(obj);
	obj->deinit();
	delete obj;
}

extern SAS_CLIENT__FUNCTION sas_Application_T SAS_CLIENT__CALL_CONVENTION sas_Server_asApplication(sas_Server_T obj)
{
	return obj;
}

extern SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_Server_run(sas_Server_T obj)
{
	assert(obj);
	obj->run();
}
