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

#include "include/sasCore/thread.h"

#include <mutex>
#include <thread>
#include <condition_variable>

namespace SAS
{
	struct Thread_priv
	{
		Thread_priv() : status(Thread::Status::NotRunning), th(nullptr)
		{ }
		std::mutex status_mutex;
		Thread::Status status;

		std::timed_mutex th_mutex;
		std::thread * th;

		static void runner(Thread * obj)
		{
			std::unique_lock<std::timed_mutex> __th_mutex_locker(obj->priv->th_mutex);
			obj->begun();
			obj->execute();
			obj->ended();
			std::unique_lock<std::mutex> __status_mutex_locker(obj->priv->status_mutex);
			obj->priv->status = Thread::Status::NotRunning;
//			delete obj->priv->th;
			obj->priv->th = nullptr;
		}
	};

	Thread::Thread() : priv(new Thread_priv)
	{ }

	Thread::~Thread()
	{
		terminate();
		delete priv;
	}

	ThreadId Thread::id()
	{
		return priv->th ? priv->th->get_id() : std::thread::id();
	}

	bool Thread::start()
	{
		std::unique_lock<std::mutex> __status_mutex_locker(priv->status_mutex);
		if(priv->status != Status::NotRunning)
			return false;
		priv->status = Status::Started;
		priv->th = new std::thread(Thread_priv::runner, this);
		return true;
	}

	void Thread::wait()
	{
		if(!priv->th)
			return;
		priv->th->join();
	}

	bool Thread::wait(long milliseconds)
	{
		if(!priv->th_mutex.try_lock_for(std::chrono::milliseconds(milliseconds)))
			return false;
		priv->th_mutex.unlock();
		return true;
	}

	void Thread::terminate()
	{
		std::unique_lock<std::mutex> __status_mutex_locker(priv->status_mutex);
		if(priv->th)
		{
			delete priv->th;
			priv->th = nullptr;
		}
	}

	void Thread::stop()
	{
		std::unique_lock<std::mutex> __status_mutex_locker(priv->status_mutex);
		priv->status = Status::Stopped;
	}

	Thread::Status Thread::status()
	{
		std::unique_lock<std::mutex> __status_mutex_locker(priv->status_mutex);
		return priv->status;
	}

	//static
	ThreadId Thread::getThreadId()
	{ 
		return std::this_thread::get_id(); 
	}

	//static 
	void Thread::sleep(long milliseconds)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
	}


	struct Notifier::Priv
	{
		std::mutex mutex;
		std::condition_variable condition;
		unsigned long count = 0;
	};

	Notifier::Notifier() : priv(new Priv)
	{ }

	Notifier::~Notifier()
	{
		delete priv;
	}

	void Notifier::notify() 
	{
		std::unique_lock<std::mutex> lock(priv->mutex);
		priv->condition.notify_one();
		++priv->count;
	}

	void Notifier::notifyAll()
	{
		std::unique_lock<std::mutex> lock(priv->mutex);
		priv->condition.notify_all();
		++priv->count;
	}

	void Notifier::wait() 
	{
		std::unique_lock<std::mutex> lock(priv->mutex);
		while (!priv->count)
			priv->condition.wait(lock);
		--priv->count;
	}

	bool Notifier::wait(long msecs)
	{
		std::unique_lock<std::mutex> lock(priv->mutex);
		while (!priv->count)
			if (priv->condition.wait_for(lock, std::chrono::milliseconds(msecs)) != std::cv_status::no_timeout)
				return false;
		--priv->count;
		return true;
	}

	bool Notifier::tryWait() 
	{
		std::unique_lock<std::mutex> lock(priv->mutex);
		if (priv->count) 
		{
			--priv->count;
			return true;
		}
		return false;
	}

}
