#ifndef sasCore__threadpool_h
#define sasCore__threadpool_h

#include "defines.h"

#include <functional>
#include <thread>
#include <string>

namespace SAS {

    class ThreadPool
    {
    public:
        class Thread
        {
            SAS_COPY_PROTECTOR(Thread)
            struct Private;
            std::unique_ptr<Private> p;

            friend class ThreadPool;

            Thread(const std::string & name);
        public:
            ~Thread();

            void run(std::function<void()> func, std::function<void()> end = nullptr);

            void join();

            bool isReleased();

            std::string name() const;

            std::thread::id id() const;
        };

        virtual inline ~ThreadPool() = default;

        virtual Thread * allocate() = 0;

        virtual void release(Thread * th) = 0;

    protected:
        Thread * makeNewThread(const std::string & name, std::string & error) const;
    };

    class SimpleThreadPool : public ThreadPool
    {
        SAS_COPY_PROTECTOR(SimpleThreadPool)
        struct Private;
        std::unique_ptr<Private> p;
    public:
        SimpleThreadPool(const std::string & name);
        virtual ~SimpleThreadPool() override;

        virtual Thread * allocate() override;

        virtual void release(Thread * th) override;

    };

}

#endif // sasCore__threadpool_h
