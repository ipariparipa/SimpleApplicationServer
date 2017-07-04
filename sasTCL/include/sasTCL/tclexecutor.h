/*
This file is part of sasTCL.

sasTCL is free software: you can redistribute it and/or modify
it under the terms of the Lesser GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

sasTCL is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with sasTCL.  If not, see <http://www.gnu.org/licenses/>
*/

#ifndef sasTCL__tclexecutor_h
#define sasTCL__tclexecutor_h

#include "config.h"
#include <sasCore/invoker.h>
#include <sasCore/thread.h>
#include SAS_TCL__TCL_H

#include <string>

namespace SAS {

	class TCLInterpInitializer;

	struct TCLExecutor_priv;

	class SAS_TCL__CLASS TCLExecutor : protected Thread
	{
	public:
		TCLExecutor(const std::string & name, Tcl_Interp * interp);
		TCLExecutor(const std::string & name, TCLInterpInitializer * initer = nullptr);

		virtual ~TCLExecutor();

		struct Run
		{
			Run() : ec(nullptr), isOK(false)
			{ }

			ErrorCollector * ec;
			std::string script;
			std::string result;
			bool isOK;
		};

		void run(Run * run);

		virtual bool start() override;

		virtual void stop() override;

	protected:
		virtual void execute() override;

	private:
		TCLExecutor_priv * priv;
	};

}

#endif //sasTCL__tclexecutor_h
