/*
    This file is part of sasCore.

    sasCore is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasCore is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasCore.  If not, see <http://www.gnu.org/licenses/>
 */

#include "include/sasCore/timerthread.h"
#include <mutex>
#include <chrono>

namespace SAS {

	struct TimerThread_priv
	{
		Notifier timer_not;
		long milliseconds;
	};

	TimerThread::TimerThread() : priv(new TimerThread_priv)
	{
		priv->timer_not.notify();
	}

	TimerThread::~TimerThread()
	{
		priv->timer_not.notifyAll();
		delete priv;
	}

	bool TimerThread::start(long milliseconds)
	{
		wait();
		priv->milliseconds = milliseconds;
		priv->timer_not.wait();
		return Thread::start();
	}

	void TimerThread::setInterval(long milliseconds)
	{
		priv->milliseconds = milliseconds;
	}

	long TimerThread::interval() const
	{
		return priv->milliseconds;
	}

	void TimerThread::stop()
	{
		Thread::stop();
		priv->timer_not.notify();
	}

	void TimerThread::begun()
	{ Thread::begun(); }

	void TimerThread::ended()
	{
		Thread::ended();
	}

	void TimerThread::execute()
	{
		while(!priv->timer_not.wait(priv->milliseconds) && status() != Thread::Status::Stopped)
			shot();
	}

}
