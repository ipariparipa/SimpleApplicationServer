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
#include "include/sasCore/notifier.h"
#include <mutex>
#include <chrono>

namespace SAS {

	struct TimerThread_priv
	{
		Notifier timer_not;
        std::chrono::milliseconds interval;
    };

    TimerThread::TimerThread(ThreadPool * pool) : Thread(pool), priv(new TimerThread_priv)
	{
		priv->timer_not.notify();
	}

	TimerThread::~TimerThread()
	{
		priv->timer_not.notifyAll();
		delete priv;
	}

    bool TimerThread::start(std::chrono::milliseconds interval)
	{
		wait();
        priv->interval = interval;
		priv->timer_not.wait();
		return Thread::start();
	}

    void TimerThread::setInterval(std::chrono::milliseconds v)
	{
        priv->interval = v;
	}

    std::chrono::milliseconds TimerThread::interval() const
	{
        return priv->interval;
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
        while(!priv->timer_not.wait(priv->interval) && status() != Thread::Status::Stopped)
			shot();
	}

}
