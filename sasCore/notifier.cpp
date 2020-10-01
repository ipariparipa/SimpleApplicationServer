
#include "include/sasCore/notifier.h"

#include <condition_variable>
#include <atomic>

namespace SAS {

struct Notifier::Priv
{
    std::mutex mutex;
    std::condition_variable condition;
	std::atomic_ulong count;

	Priv()
	{ 
		count = 0;
    }

};

Notifier::Notifier() : p(new Priv)
{}

Notifier::~Notifier()
{
	delete p;
}

void Notifier::notify()
{
    std::unique_lock<std::mutex> lock(p->mutex);
    p->condition.notify_one();
    ++p->count;
}

void Notifier::notifyAll()
{
    std::unique_lock<std::mutex> lock(p->mutex);
    p->condition.notify_all();
    ++p->count;
}

void Notifier::wait()
{
    std::unique_lock<std::mutex> lock(p->mutex);
    while (!p->count)
         p->condition.wait(lock);
    --p->count;
}

bool Notifier::wait(std::chrono::milliseconds msecs)
{
    std::unique_lock<std::mutex> lock(p->mutex);
    while (!p->count)
         if (p->condition.wait_for(lock, msecs) != std::cv_status::no_timeout)
                 return false;
    --p->count;
    return true;
}

bool Notifier::wait(std::chrono::time_point<std::chrono::system_clock> time)
{
    std::unique_lock<std::mutex> lock(p->mutex);
    while (!p->count)
         if (p->condition.wait_until(lock, time) != std::cv_status::no_timeout)
                 return false;
    --p->count;
    return true;
}

#if SAS_OS != SAS_OS_WINDOWS
bool Notifier::wait(std::chrono::time_point<std::chrono::steady_clock> time)
{
    std::unique_lock<std::mutex> lock(p->mutex);
    while (!p->count)
         if (p->condition.wait_until(lock, time) != std::cv_status::no_timeout)
                 return false;
    --p->count;
    return true;
}
#endif

bool Notifier::tryWait()
{
    std::unique_lock<std::mutex> lock(p->mutex);
    if (p->count)
    {
         --p->count;
         return true;
    }
    return false;
}

}
