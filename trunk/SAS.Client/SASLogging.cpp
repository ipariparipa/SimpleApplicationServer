/*
This file is part of SAS.Client.

SAS.Client is free software: you can redistribute it and/or modify
it under the terms of the Lesser GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

SAS.Client is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with SAS.Client.  If not, see <http://www.gnu.org/licenses/>
*/

#include "SASLogging.h"
#include <sasBasics/logging.h>

#include "SASErrorCollector.h"

#include "macros.h"

namespace SAS { namespace Client {

	//static 
	bool SASLogging::Init(array<System::String^> ^ argv, ISASErrorCollector ^ ec)
	{
		std::vector<std::string> _argv_buffer(argv->Length);
		std::vector<char*> _argv(argv->Length);
		for (int i = 0, l = argv->Length; i < l; ++i)
		{
			auto & s = _argv_buffer[i];
			auto str = argv[i];
			s = TO_STR(str);
			_argv[i] = (char*)s.c_str();
		}

		WErrorCollector _ec(ec);
		return Logging::init(_argv.size(), _argv.data(), _ec);
	}

}}
