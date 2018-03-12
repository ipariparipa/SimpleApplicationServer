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

	struct TCLExecutor::Priv
	{
		Priv(const std::string & name, Tcl_Interp * interp_) :
			current_run(nullptr),
			interp(interp_),
			logger(Logging::getLogger("SAS.TCLExecutor." + name))
		{ }

		Notifier run_not, start_not;

		std::mutex run_mut;
		TCLExecutor::Run * current_run;

		Tcl_Interp * interp;

		Logging::LoggerPtr logger;
	};

	TCLExecutor::TCLExecutor(const std::string & name, Tcl_Interp * interp) : 
		ControlledThread(), 
		priv(new Priv(name, interp))
	{ }

	TCLExecutor::~TCLExecutor()
	{
		stop();
		resume();
		if (!wait(SAS_TCL__EXECUTION_TIMEOUT))
			terminate();
		delete priv;
	}

	bool TCLExecutor::run(Run * run, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		{
			std::unique_lock<std::mutex> __run_locker(priv->run_mut);
			priv->current_run = run;
			resume();
		}
		if (!priv->run_not.wait(SAS_TCL__EXECUTION_TIMEOUT))
		{
			auto err = ec.add(SAS_TCL__ERROR__RUN_TIMEOUT, "unexpected error: TCL executor timeout");
			SAS_LOG_FATAL(priv->logger, err);
			return false;
		}
		return true;
	}

	bool TCLExecutor::start()
	{
		NullEC ec;
		return start(ec);
	}

	bool TCLExecutor::start(ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		if (!Thread::start())
		{
			auto err = ec.add(SAS_TCL__ERROR__UNEXPECTED, "could not start TCL executor thread");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}
		SAS_LOG_TRACE(priv->logger, "waiting for executor thread is started");
		if (!priv->start_not.wait(SAS_TCL__START_TIMEOUT))
		{
			auto err = ec.add(SAS_TCL__ERROR__UNEXPECTED, "unexpected error: TCL executor starting timeout");
			SAS_LOG_FATAL(priv->logger, err);
			return false;
		}
		SAS_LOG_TRACE(priv->logger, "executor thread has been started properly");
		return true;
	}

	void TCLExecutor::stop()
	{
		Thread::stop();
	}

	void TCLExecutor::execute()
	{
		SAS_LOG_NDC();
		Tcl_Interp * _local_interp = nullptr;
		Tcl_Interp * _interp;
		if (priv->interp)
			_interp = priv->interp;
		else
		{
			SAS_LOG_TRACE(priv->logger, "Tcl_CreateInterp");
			_interp = _local_interp = Tcl_CreateInterp();
		}
		assert(_interp);

		priv->start_not.notifyAll();

		SAS_LOG_TRACE(priv->logger, "enter to worker loop");
		while (enterContolledSection() && status() != Thread::Status::Stopped)
		{
			std::unique_lock<std::mutex> __run_locker(priv->run_mut);

			if (priv->current_run)
			{
				switch (priv->current_run->operation)
				{
				case Run::Exec:
					SAS_LOG_TRACE(priv->logger, "Tcl_GlobalEval");
					if (Tcl_GlobalEval(_interp, priv->current_run->script.c_str()) == TCL_ERROR)
					{
						TCLListHandler lst(_interp, Tcl_GetObjResult(_interp));

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
						priv->current_run->result = Tcl_GetStringResult(_interp);
						priv->current_run->isOK = true;
					}
					break;
				case Run::Init:
					if (priv->current_run->initer)
					{
						SAS_LOG_TRACE(priv->logger, "call initer");
						priv->current_run->initer->init(_interp);
					}
					break;
				}

			}
			priv->current_run = nullptr;
			suspend();
			priv->run_not.notify();
		}
		SAS_LOG_TRACE(priv->logger, "worker loop is ended");

		if (_local_interp)
		{
			SAS_LOG_TRACE(priv->logger, "Tcl_DeleteInterp");
			Tcl_DeleteInterp(_local_interp);
		}
	}

	struct TCLExecutorPool::Priv
	{
		Priv(const std::string & name_, Tcl_Interp * interp_) :
			name(name_),
			interp(interp_),
			logger(Logging::getLogger("SAS.TCLExecutorPool." + name_))
		{ }

		std::mutex mut;
		std::map<TCLExecutor*, size_t /*used*/> pool;

		std::string name;
		Tcl_Interp * interp;
		Logging::LoggerPtr logger;

	};

	TCLExecutorPool::TCLExecutorPool(const std::string & name, Tcl_Interp * interp) : priv(new Priv(name, interp))
	{ }

	TCLExecutorPool::~TCLExecutorPool()
	{
		{
			std::unique_lock<std::mutex> __locker(priv->mut);
			for (auto & o : priv->pool)
				if (!o.second)
					delete o.first;
		}

		delete priv;
	}

	TCLExecutor * TCLExecutorPool::consume(ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		std::unique_lock<std::mutex> __locker(priv->mut);
		for (auto & o : priv->pool)
			if (o.second == 0)
			{
				o.second = true;
				SAS_LOG_TRACE(priv->logger, "reuse existing TCLExecutor");
				return o.first;
			}

		SAS_LOG_TRACE(priv->logger, "no free executor is found in pool");

		TCLExecutor * ret;
		//if (priv->pool.size() >= SAS_TCL__MAX_EXECUTORS)
		//{
		//	SAS_LOG_TRACE(priv->logger, "maximum number of executors is reached, select one to reuse");
		//	ret = nullptr;
		//	size_t tmp = 0;
		//	for (auto & o : priv->pool)
		//		if (o.second < tmp || tmp == 0)
		//		{
		//			tmp = o.second;
		//			ret = o.first;
		//		}
		//}
		//else
		{
			SAS_LOG_TRACE(priv->logger, "create new TCLExecutor");
			ret = new TCLExecutor(priv->name, priv->interp);
			SAS_LOG_TRACE(priv->logger, "start TCL executor thread");
			if (!ret->start(ec))
			{
				delete ret;
				return nullptr;
			}
		}

		SAS_LOG_ASSERT(priv->logger, ret, "executor object must be existing/created");

		++priv->pool[ret];
		return ret;
	}

	void TCLExecutorPool::release(TCLExecutor * exec)
	{
		SAS_LOG_NDC();
		std::unique_lock<std::mutex> __locker(priv->mut);
		if (!priv->pool.count(exec))
		{
			SAS_LOG_WARN(priv->logger, "unexpected: executor is not found in pool");
			return;
		}
		auto & cnt = priv->pool[exec];
		if (cnt == 0)
		{
			SAS_LOG_WARN(priv->logger, "unexpected: executor is already free");
			return;
		}
		--cnt;
	}
}
