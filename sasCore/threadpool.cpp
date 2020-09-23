#include "include/sasCore/threadpool.h"
#include "include/sasCore/notifier.h"
#include "include/sasCore/logging.h"

#include <thread>
#include <atomic>
#include <mutex>
#include <list>
#include <set>
#include "assert.h"

namespace SAS {

    struct ThreadPool::Thread::Private
    {
        Private(const std::string & name) :
            name(name),
            logger(SAS::Logging::getLogger(name)),
            running(true),
            released(true),
            thread(new std::thread([this]() {
                while(true)
                {
                    notif.wait();
                    {
                        std::unique_lock<std::mutex> __locker(flag_mut);
                        if(!running)
                            break;
                        released = false;
                    }
                    std::unique_lock<std::mutex> __locker(func_mut);
                    if(func)
                        func();
                    {
                        std::unique_lock<std::mutex> __locker(flag_mut);
                        if(!running)
                            break;
                        released = true;
                    }
                    if(end)
                        end();
                }
            }))
        { }

        ~Private()
        {
            bool has_to_join = false;
            {
                std::unique_lock<std::mutex> __locker(flag_mut);
                running = false;
                if(released)
                {
                    has_to_join = true;
                    notif.notify();
                }
            }
            if(has_to_join)
                thread->join();
        }

        std::string name;
        Logging::LoggerPtr logger;

        std::mutex flag_mut;
        bool running;
        bool released;

        Notifier notif;
        std::thread * thread;
        std::mutex func_mut;
        std::function<void()> func;
        std::function<void()> end;
    };

    ThreadPool::Thread::Thread(const std::string & name) : p(new Private(name))
    { }

    ThreadPool::Thread::~Thread() = default;

    void ThreadPool::Thread::run(std::function<void()> func, std::function<void()> end)
    {
        std::unique_lock<std::mutex> __locker(p->func_mut);
        p->func = func;
        p->end = end;
        p->notif.notify();
    }

    void ThreadPool::Thread::join()
    {
        p->func_mut.lock();
        p->func_mut.unlock();
    }

    bool ThreadPool::Thread::isReleased()
    {
        return p->released;
    }

    std::string ThreadPool::Thread::name() const
    {
        return p->name;
    }

    std::thread::id ThreadPool::Thread::id() const
    {
        return p->thread->get_id();
    }


    ThreadPool::Thread * ThreadPool::makeNewThread(const std::string & name, std::string & error) const
    {
        try {
           return new Thread(name);
        }
        catch(const std::system_error & e) {
            error = "system error: " + std::string(e.what());
        }

        return nullptr;
    }

    struct SimpleThreadPool::Private
    {
        Private(const std::string & name) :
            logger(Logging::getLogger(name))
        { }

        Logging::LoggerPtr logger;
        std::mutex mut;
        std::list<std::unique_ptr<Thread>> threads;
        std::set<Thread*> freeThreads;
    };

    SimpleThreadPool::SimpleThreadPool(const std::string & name) : p(new Private(name))
    { }

    SimpleThreadPool::~SimpleThreadPool() = default;

    SimpleThreadPool::Thread * SimpleThreadPool::allocate()
    {
        std::unique_lock<std::mutex> __locker(p->mut);
        if(p->freeThreads.size())
        {
            for(auto & th : p->freeThreads)
            {
                if(th->isReleased())
                {
                    p->freeThreads.erase(th);
                    SAS_LOG_TRACE(p->logger, "allocate thread: '"+th->name() + "'");
                    return th;
                }
            }
        }

        Thread * ret;
        auto thread_name = "thread#" + std::to_string(p->threads.size() + 1);
        SAS_LOG_DEBUG(p->logger, "creating new thread '" + thread_name + "'");
        std::string err;
        if((ret = makeNewThread(thread_name, err)))
            p->threads.push_back(std::unique_ptr<Thread>(ret));
        else
            SAS_LOG_ERROR(p->logger, "could not create thead in thread pool: '"+err+"'");

        return ret;
    }

    void SimpleThreadPool::release(SimpleThreadPool::Thread * th)
    {
        if(th)
        {
            std::unique_lock<std::mutex> __locker(p->mut);
            if(!p->freeThreads.count(th))
            {
                SAS_LOG_TRACE(p->logger, "release thread: '"+th->name() + "'");
                p->freeThreads.insert(th);
            }
        }
    }

}
