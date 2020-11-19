
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
            using Ptr = std::shared_ptr<Entry>;
			Id id = 0;
			std::chrono::system_clock::time_point timestamp;
            std::chrono::system_clock::duration period;
            long remaining = 1;
			Func func;
            bool done = false;
            std::mutex mut;
        };

		std::mutex entries_mut;
        std::map<std::chrono::system_clock::time_point, std::list<Entry::Ptr>> entries;
        Entry::Ptr current_entry;

        Entry::Ptr takeNext()
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

            return current_entry = nullptr;
		}

		void restore()
		{
			std::unique_lock<std::mutex> __locker(entries_mut);
            if (current_entry)
            {
                std::unique_lock<std::mutex> __e_locker(current_entry->mut);
                entries[current_entry->timestamp].push_back(current_entry);
            }
		}

        Entry::Ptr current()
		{
			std::unique_lock<std::mutex> __locker(entries_mut);
			return current_entry;
		}


        void add(TimelineThread::Id id, std::chrono::system_clock::time_point timestamp, std::chrono::system_clock::duration period, Func func, long cycle)
        {
            std::unique_lock<std::mutex> __locker(entries_mut);

            auto e = std::make_shared<Private::Entry>();
            e->id = id;
            e->func = func;
            e->timestamp = timestamp;
            e->period = period;
            e->remaining = cycle;

            entries[e->timestamp].push_back(e);
            notif.notify();
        }

	};

    TimelineThread::TimelineThread(ThreadPool * pool) : ControlledThread(pool), p(new Private)
	{ }

    TimelineThread::~TimelineThread()
    {
        wait();
    }

	void TimelineThread::stop() // final override
	{
		ControlledThread::stop();
        resume();
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
        if(p->current_entry)
        {
            std::unique_lock<std::mutex> __entry_locker(p->current_entry->mut);
            auto __ce = p->current_entry;
            if (p->current_entry->id == id)
            {
                p->current_entry->remaining = 0;
                p->current_entry = nullptr;
            }
        }
        else
        {
            for (auto it = p->entries.begin(); it != p->entries.end(); ++it)
            {
                for (auto it_l = it->second.begin(); it_l != it->second.end(); ++it_l)
                {
                    if ((*it_l)->id == id)
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
            if(status() == Status::Stopped)
                break;

            auto e = p->takeNext();

            if (!e)
				suspend();
			else
			{
                std::chrono::system_clock::time_point timestamp;
                {
                    std::unique_lock<std::mutex> __e_locker(e->mut);
                    if (std::chrono::system_clock::now() >= e->timestamp)
                    {
                        e->func(e->id);
                        if(e->remaining == -1 || --e->remaining > 0)
                            p->add(e->id, e->timestamp + e->period, e->period, e->func, e->remaining);
                    }
                    else
                        timestamp = e->timestamp;
                }
                if(timestamp != std::chrono::system_clock::time_point())
                {
                    if (p->notif.wait(timestamp))
                    {
                        p->restore();
                        continue;
                    }
                    else
                    {
                        std::unique_lock<std::mutex> __e_locker(e->mut);
                        e->func(e->id);
                        if(e->remaining == -1 || --e->remaining > 0)
                            p->add(e->id, e->timestamp + e->period, e->period, e->func, e->remaining);
                    }
                }
			}
		}
	}

}
