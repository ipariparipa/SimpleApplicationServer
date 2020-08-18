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
#include SAS_TCL__TCL_H

#include <string>

#include <sasCore/controlledthread.h>

namespace SAS {

	class TCLInterpInitializer;

	class SAS_TCL__CLASS TCLExecutor : protected ControlledThread
	{
		struct Priv;
		Priv * priv;
	public:
        TCLExecutor(ThreadPool * pool, const std::string & name, Tcl_Interp * interp = nullptr);

        virtual ~TCLExecutor() override;

		struct Run
		{
			ErrorCollector * ec = nullptr;

			enum Operation
			{
				Exec,
				Init,
			} operation = Exec;

			TCLInterpInitializer * initer = nullptr;

			std::string script;
			std::string result;
			bool isOK = false;
		};

		bool run(Run * run, ErrorCollector & ec);

		bool start(ErrorCollector & ec);

		virtual void stop() override;

	protected:
		virtual void execute() override;


	private:
		virtual bool start() override;


	};

	class SAS_TCL__CLASS TCLExecutorPool
	{
		struct Priv;
		Priv * priv;
	public:
        TCLExecutorPool(ThreadPool * pool, const std::string & name, Tcl_Interp * interp = nullptr);

		~TCLExecutorPool();

		TCLExecutor * consume(ErrorCollector & ec);
		void release(TCLExecutor * exec);

	};

}

#endif //sasTCL__tclexecutor_h
