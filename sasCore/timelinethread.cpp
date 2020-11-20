
#include "include/sasCore/timelinethread.h"

#include <mutex>
#include <map>
#include <list>
#include <atomic>
#include <assert.h>

#include <include/sasCore/notifier.h>

#include <iostream>

namespace SAS {

    struct TimelineThread::Private
	{
		Notifier notif;

        struct Entry : public TimelineThread::Entry
		{
            using Ptr = std::shared_ptr<Entry>;
            Id id = 0;
            std::chrono::system_clock::time_point m_timestamp;
            std::chrono::system_clock::duration m_period;
            long m_remaining = 1;
			Func func;
            std::string m_handlerDescriptor;
            bool done = false;
            std::recursive_mutex mut;
            std::atomic_bool active;
            std::function<void(const Entry::Ptr &)> onChanged;

            void callOnChanged(const Entry::Ptr & e)
            {
                std::unique_lock<std::recursive_mutex> __locaker(mut);
                if(onChanged)
                    onChanged(e);
            }

            std::chrono::system_clock::time_point timestamp() final override
            {
                std::unique_lock<std::recursive_mutex> __locaker(mut);
                return m_timestamp;
            }

            std::chrono::system_clock::duration period() final override
            {
                std::unique_lock<std::recursive_mutex> __locaker(mut);
                return m_period;
            }

            long remaining() final override
            {
                std::unique_lock<std::recursive_mutex> __locaker(mut);
                return m_remaining;
            }

            std::string handlerDescriptor() final override
            {
                std::unique_lock<std::recursive_mutex> __locaker(mut);
                return m_handlerDescriptor;
            }

            void setOnChanged(std::function<void(const TimelineThread::Entry::Ptr &)> func) final override
            {
                std::unique_lock<std::recursive_mutex> __locaker(mut);
                onChanged = func;
            }

        };

        std::mutex onChanged_mut;
        std::function<void(const std::vector<TimelineThread::Entry::Ptr> &)> onChanged;
        void callOnChanged()
        {
            std::unique_lock<std::mutex> __locker(onChanged_mut);
            if(onChanged)
                onChanged(getEntries());
        }

        std::recursive_mutex entries_mut;
        std::map<std::chrono::system_clock::time_point, std::list<Entry::Ptr>> entries;
        Entry::Ptr current_entry;

        std::vector<TimelineThread::Entry::Ptr> getEntries()
        {
            std::unique_lock<std::recursive_mutex> __locaker(entries_mut);

            size_t cnt = 0;
            for(auto & t : entries)
                cnt += t.second.size();

            std::vector<TimelineThread::Entry::Ptr> ret(cnt);
            size_t i = 0;
            for(auto & t : entries)
                for(auto & e : t.second)
                {
                    assert(i < cnt);
                    ret[i++] = e;
                }

            return ret;
        }

        Entry::Ptr takeNext()
		{
            std::unique_lock<std::recursive_mutex> __locker(entries_mut);

			while (entries.size())
			{
				auto it = entries.begin();
				if (it->second.size())
                {
                    current_entry = it->second.front(); // the next one
                    it->second.pop_front();
                    callOnChanged();
					return current_entry;
				}
				else
                    entries.erase(it); // cleaning up
            }

            return current_entry = nullptr;
		}

		void restore()
        { // inserts back the current entry
            std::unique_lock<std::recursive_mutex> __locker(entries_mut);
            if (current_entry)
            {
                std::unique_lock<std::recursive_mutex> __e_locker(current_entry->mut);
                entries[current_entry->m_timestamp].push_back(current_entry);
            }
		}

        Entry::Ptr current()
		{
            std::unique_lock<std::recursive_mutex> __locker(entries_mut);
			return current_entry;
		}


        void add(TimelineThread::Id id, std::chrono::system_clock::time_point timestamp, std::chrono::system_clock::duration period, Func func, const std::string & handlerDescriptor, long cycle, std::function<void(Entry::Ptr)> onChanged = nullptr)
        {
            auto e = std::make_shared<Private::Entry>();
            e->id = id;
            e->func = func;
            e->m_timestamp = timestamp;
            e->m_period = period;
            e->m_remaining = cycle;
            e->m_handlerDescriptor = handlerDescriptor;
            e->active = true;
            e->onChanged = onChanged;

            add(e);
            e->callOnChanged(e);
        }

        void add(const Entry::Ptr & e)
        {
            std::unique_lock<std::recursive_mutex> __locker(entries_mut);

            entries[e->m_timestamp].push_back(e);

            callOnChanged();
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

    TimelineThread::Id TimelineThread::add(std::chrono::system_clock::duration timeout, Func func, const std::string & handlerDescriptor, long cycle, std::function<void(const Entry::Ptr &)> onChanged)
    {
        return add(std::chrono::system_clock::now() + timeout, timeout, func, handlerDescriptor, cycle, onChanged);
    }

    TimelineThread::Id TimelineThread::add(std::chrono::system_clock::time_point timestamp, Func func, const std::string & handlerDescriptor, std::function<void(const Entry::Ptr &)> onChanged)
    {
        return add(timestamp, std::chrono::system_clock::duration(), func, handlerDescriptor, 1, onChanged);
    }

    TimelineThread::Id TimelineThread::add(std::chrono::system_clock::time_point timestamp, std::chrono::system_clock::duration period, Func func, const std::string & handlerDescriptor, long cycle, std::function<void(const Entry::Ptr &)> onChanged)
    {
        std::mutex mut;
        std::unique_lock<std::mutex> __locker(mut);
        static Id id(0);

        p->add(++id, timestamp, period, func, handlerDescriptor, cycle, onChanged);

        resume();

        return id;
	}

	void TimelineThread::cancel(TimelineThread::Id id)
	{
        std::unique_lock<std::recursive_mutex> __locker(p->entries_mut);
        if(p->current_entry)
        {
            p->current_entry->active = false; //deactivate the entry, when already/still in use
            if(p->current_entry->mut.try_lock())
            {
                if (p->current_entry->id == id)
                {
                    p->current_entry->m_remaining = 0;
                    p->current_entry->mut.unlock();
                    p->current_entry = nullptr;
                    p->notif.notify();
                    return;
                }
                p->current_entry->mut.unlock();
            }
        }

        for (auto it = p->entries.begin(); it != p->entries.end(); ++it)
        {
            for (auto it_l = it->second.begin(); it_l != it->second.end(); ++it_l)
            {
                if ((*it_l)->id == id)
                {
                    it->second.erase(it_l);
                    (*it_l)->callOnChanged(nullptr);
                    if (!it->second.size())
                        p->entries.erase(it);

                    p->callOnChanged();
                    return;
                }
            }
        }
	}

    void TimelineThread::setOnChanged(std::function<void(const std::vector<Entry::Ptr> &)> func)
    {
        std::unique_lock<std::mutex> __locker(p->onChanged_mut);
        p->onChanged = func;
    }

    bool TimelineThread::setOnChanged(Id id, std::function<void(const Entry::Ptr &)> func)
    {
        std::unique_lock<std::recursive_mutex> __locker(p->entries_mut);


        for(auto & t : p->entries)
            for(auto & e : t.second)
                if(e->id == id)
                {
                    e->setOnChanged(func);
                    return true;
                }

        return false;
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
            else if(e->active)
			{
                std::chrono::system_clock::time_point timestamp; // the time point to be waiting for
                {
                    std::unique_lock<std::recursive_mutex> __e_locker(e->mut);
                    if (std::chrono::system_clock::now() >= e->m_timestamp)
                    { // the entry was relevant in the past
                        e->func(e->id, e);
                        if(e->m_remaining == -1 || --e->m_remaining > 0)
                        {
                            e->m_timestamp += e->m_period;
                            p->add(e); // append a new repetitive entry to the timeline
                            e->callOnChanged(e);
                        }
                        else
                            e->callOnChanged(nullptr);
                    }
                    else
                        timestamp = e->m_timestamp;
                }
                if(timestamp != std::chrono::system_clock::time_point())
                { // has to wait for
                    if (p->notif.wait(timestamp)) // waiting for the time point of next entry
                    { // something happened in the timeline, so let's do again
                        p->restore();
                        //e->callOnChanged(e); // did not changed
                        p->callOnChanged();
                        continue;
                    }
                    else if(e->active)
                    { // next time point reached
                        std::unique_lock<std::recursive_mutex> __e_locker(e->mut);
                        e->func(e->id, e);
                        if(e->m_remaining == -1 || --e->m_remaining > 0)
                        {
                            e->m_timestamp += e->m_period;
                            p->add(e); // append a new repetitive entry to the timeline
                            e->callOnChanged(e);
                        }
                        else
                            e->callOnChanged(nullptr);
                    }
                }
			}
		}
	}

    TimelineThread::Entry::Ptr TimelineThread::entry(Id id)
    {
        std::unique_lock<std::recursive_mutex> __locaker(p->entries_mut);

        for(auto & t : p->entries)
            for(auto & e : t.second)
                if(e->id == id)
                    return e;

        return nullptr;
    }

    std::vector<TimelineThread::Entry::Ptr> TimelineThread::entries()
    {
        return p->getEntries();
    }

}
