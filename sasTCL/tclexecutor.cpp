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

#include "include/sasTCL/tclexecutor.h"
#include "include/sasTCL/tcllisthandler.h"
#include "include/sasTCL/tclinterpinitilizer.h"
#include "include/sasTCL/errorcodes.h"

#include <sasCore/logging.h>
#include <sasCore/errorcollector.h>

#include <mutex>
#include <condition_variable>

namespace SAS {

	struct TCLExecutor_priv
	{
		TCLExecutor_priv(const std::string & name, Tcl_Interp * interp_, TCLInterpInitializer * initer_) :
			current_run(nullptr), interp(interp_), initer(initer_), logger(Logging::getLogger("SAS.TCLExecutor." + name))
		{ }

		std::mutex susp_mut, wait_mut, run_mut;
		std::condition_variable susp_cv, wait_cv, run_cv;

		TCLExecutor::Run * current_run;

		Tcl_Interp * interp;
		TCLInterpInitializer * initer;

		Logging::LoggerPtr logger;
	};

	TCLExecutor::TCLExecutor(const std::string & name, Tcl_Interp * interp) : Thread(), priv(new TCLExecutor_priv(name, interp, nullptr))
	{ }

	TCLExecutor::TCLExecutor(const std::string & name, TCLInterpInitializer * initer) : Thread(), priv(new TCLExecutor_priv(name, nullptr, initer))
	{ }

	TCLExecutor::~TCLExecutor()
	{
		stop();
		wait();
		delete priv;
	}

	void TCLExecutor::run(TCLExecutor::Run * run)
	{
		std::unique_lock<std::mutex> __waiter_locker(priv->wait_mut);
		{
			std::lock_guard<std::mutex> __susp_locker(priv->susp_mut);
			priv->current_run = run;
			priv->susp_cv.notify_one();
		}
		priv->wait_cv.wait(__waiter_locker);
	}

	bool TCLExecutor::start()
	{
		std::unique_lock<std::mutex> __run_locker(priv->run_mut);
		if (!Thread::start())
			return false;
		priv->run_cv.wait(__run_locker);
		return true;
	}

	void TCLExecutor::stop()
	{
		Thread::stop();
		std::lock_guard<std::mutex> __susp_locker(priv->susp_mut);
		priv->susp_cv.notify_one();
	}

	void TCLExecutor::execute()
	{
		Tcl_Interp * _my_interp = nullptr;
		if (!priv->interp)
		{
			SAS_LOG_NDC();
			SAS_LOG_TRACE(priv->logger, "Tcl_CreateInterp");
			priv->interp = _my_interp = Tcl_CreateInterp();

			if (priv->initer)
				priv->initer->init(priv->interp);
		}

		while (true)
		{
			std::unique_lock<std::mutex> __run_locker(priv->run_mut);
			std::unique_lock<std::mutex> __susp_locker(priv->susp_mut);
			__run_locker.unlock();
			priv->run_cv.notify_one();
			priv->susp_cv.wait(__susp_locker);
			if (status() == Thread::Status::Stopped)
				break;
			std::lock_guard<std::mutex> __wait_locker(priv->wait_mut);

			if (priv->current_run)
			{
				SAS_LOG_TRACE(priv->logger, "Tcl_GlobalEval");

				if (Tcl_GlobalEval(priv->interp, priv->current_run->script.c_str()) == TCL_ERROR)
				{
					TCLListHandler lst(priv->interp, Tcl_GetObjResult(priv->interp));

					for (int i(0), l(lst.length()); i < l; ++i)
					{
						auto err = lst.getList(i);
						if (err.length() == 2)
							priv->current_run->ec->add(std::stol(err[0]), err[1]);
						else
						{
							priv->current_run->ec->add(SAS_TCL__ERROR__EXECUTOR__CANNOT_RUN_SCRIPT, "could not run script: '" + lst.toString() + "'");
							break;
						}
					}
					priv->current_run->isOK = false;
				}
				else
				{
					std::string res;
					priv->current_run->result = Tcl_GetStringResult(priv->interp);
					priv->current_run->isOK = true;
				}
			}
			priv->current_run = nullptr;
			priv->wait_cv.notify_one();
		}


		if (_my_interp)
		{
			SAS_LOG_TRACE(priv->logger, "Tcl_DeleteInterp");
			Tcl_DeleteInterp(priv->interp);
			priv->interp = nullptr;
		}
	}
}
