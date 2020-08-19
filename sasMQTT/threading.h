/*
This file is part of sasMQTT.

sasMQTT is free software: you can redistribute it and/or modify
it under the terms of the Lesser GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

sasMQTT is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with sasMQTT.  If not, see <http://www.gnu.org/licenses/>
*/

#ifndef sasMQTT__threading_h
#define sasMQTT__threading_h

#include "include/sasMQTT/config.h"

#include <sasCore/controlledthread.h>

#include <mutex>
#include <memory>
#include <deque>
#include <list>

namespace SAS {

	template <typename T_Task>
	class TaskQueue
	{
		std::mutex mut;
		std::deque<T_Task * > data;
	public:
		void add(T_Task * run)
		{
			std::unique_lock<std::mutex> __locker(mut);

			data.push_back(run);
		}

		T_Task * take()
		{
			std::unique_lock<std::mutex> __locker(mut);

			if (!data.size())
				return nullptr;
			auto d = data.front();
			data.pop_front();
			return d;
		}

		std::list<T_Task *> takeAll()
		{
			std::unique_lock<std::mutex> __locker(mut);

			std::list<T_Task *> tmp;
			std::copy(data.begin(), data.end(), tmp.begin());
			data.clear();
			return tmp;
		}
	};


	template<typename T_task>
	class ThreadInPool : public ControlledThread
	{
	public:
        ThreadInPool(ThreadPool * pool) : ControlledThread(pool)
		{ }

		virtual ~ThreadInPool() { }

		void add(T_task * task)
		{
			_tasks.add(task);
			resume();
		}

	protected:
		virtual void execute() final
		{
			while(enterContolledSection())
			{
				auto task = _tasks.take();
				if(!task)
				{
					suspend();
					continue;
				}
				complete(task);
				delete task;
			}
		}

		virtual bool complete(T_task * task) = 0;

	private:
		TaskQueue<T_task> _tasks;
	};


	template<typename T_Thread>
	class AbstractThreadPool
	{
		std::mutex mut;
		std::list<std::shared_ptr<T_Thread>> threads;
	public:
		virtual ~AbstractThreadPool() { }

		std::shared_ptr<T_Thread> get()
		{
			std::unique_lock<std::mutex> __locker(mut);

			for (auto & th : threads)
			{
				if (th->reserveSuspended())
					return th;
			}
			std::shared_ptr<T_Thread> th;
			threads.push_back(th = newThread());
			th->start();
			//th->suspend();
			return th;
		}
	protected:
		virtual std::shared_ptr<T_Thread> newThread() = 0;
	};

}

#endif // sasMQTT__threading_h
