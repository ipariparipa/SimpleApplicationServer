
#include "include/sasCore/timelinethread.h"

#include <mutex>
#include <map>
#include <list>
#include <atomic>

#include <include/sasCore/notifier.h>

namespace SAS {

    struct TimelineThread::Private
	{
		Notifier notif;

		struct Entry
		{
			Id id = 0;
			std::chrono::system_clock::time_point timestamp;
            std::chrono::system_clock::duration period;
            long remaining = 1;
			Func func;
			bool done = false;
		};

		std::mutex entries_mut;
		std::map<std::chrono::system_clock::time_point, std::list<Entry>> entries;
		Entry current_entry;

		Entry takeNext()
		{
			std::unique_lock<std::mutex> __locker(entries_mut);

			while (entries.size())
			{
				auto it = entries.begin();
				if (it->second.size())
                {
					current_entry = it->second.front();
					it->second.pop_front();
					return current_entry;
				}
				else
                    entries.erase(it);
            }

			return current_entry = Entry();
		}

		void restore()
		{
			std::unique_lock<std::mutex> __locker(entries_mut);
			if (current_entry.id)
				entries[current_entry.timestamp].push_back(current_entry);
		}

		Entry current()
		{
			std::unique_lock<std::mutex> __locker(entries_mut);
			return current_entry;
		}


        void add(TimelineThread::Id id, std::chrono::system_clock::time_point timestamp, std::chrono::system_clock::duration period, Func func, long cycle)
        {
            std::unique_lock<std::mutex> __locker(entries_mut);

            Private::Entry e;
            e.id = id;
            e.func = func;
            e.timestamp = timestamp;
            e.period = period;
            e.remaining = cycle;

            entries[e.timestamp].push_back(e);
            notif.notify();
        }

	};

    TimelineThread::TimelineThread() : ControlledThread(), p(new Private)
	{ }

    TimelineThread::~TimelineThread() = default;

	void TimelineThread::stop() // final override
	{
		ControlledThread::stop();
		p->notif.notifyAll();
	}

    TimelineThread::Id TimelineThread::add(std::chrono::system_clock::duration timeout, Func func, long cycle)
    {
        return add(std::chrono::system_clock::now() + timeout, timeout, func, cycle);
    }

    TimelineThread::Id TimelineThread::add(std::chrono::system_clock::time_point timestamp, Func func)
    {
        return add(timestamp, std::chrono::system_clock::duration(), func, 1);
    }

    TimelineThread::Id TimelineThread::add(std::chrono::system_clock::time_point timestamp, std::chrono::system_clock::duration period, Func func, long cycle)
    {
        std::mutex mut;
        std::unique_lock<std::mutex> __locker(mut);
        static Id id(0);

        p->add(++id, timestamp, period, func, cycle);

        resume();

        return id;
	}

	void TimelineThread::cancel(TimelineThread::Id id)
	{
		std::unique_lock<std::mutex> __locker(p->entries_mut);
		if (p->current_entry.id && p->current_entry.id == id)
        {
            p->current_entry = Private::Entry();
		}
		else
		{
			for (auto it = p->entries.begin(); it != p->entries.end(); ++it)
			{
				for (auto it_l = it->second.begin(); it_l != it->second.end(); ++it_l)
				{
					if (it_l->id == id)
					{
						it->second.erase(it_l);
						if (!it->second.size())
                            p->entries.erase(it);
						return;
					}
				}
			}
		}

		p->notif.notify();
	}

	void TimelineThread::execute() //final override
	{
        while (enterContolledSection())
		{
            Private::Entry e = p->takeNext();

            if (!e.id)
				suspend();
			else
			{
                if (std::chrono::system_clock::now() >= e.timestamp)
                {
					e.func(e.id);
                    if(e.remaining == -1 || --e.remaining > 0)
                        p->add(e.id, e.timestamp + e.period, e.period, e.func, e.remaining);
                }
                else if (p->notif.wait(e.timestamp))
				{
                    p->restore();
                    continue;
				}
				else
                {
					e.func(e.id);
                    if(e.remaining == -1 || --e.remaining > 0)
                        p->add(e.id, e.timestamp + e.period, e.period, e.func, e.remaining);
                }
			}
		}
	}

}
