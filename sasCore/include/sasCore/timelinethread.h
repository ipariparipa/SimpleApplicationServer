
#ifndef SAS_CORE__TIMELINETHREAD_H
#define SAS_CORE__TIMELINETHREAD_H

#include "controlledthread.h"
#include <chrono>
#include <functional>
#include <memory>

namespace SAS {

    class TimelineThread : public SAS::ControlledThread
	{
        struct Private;
        std::unique_ptr<Private> p;

	public:

		using Id = unsigned long long;

		using Func = std::function<void(Id id)>;


		TimelineThread();
		virtual ~TimelineThread() override;

		void stop() final override;

		template<typename DurationT>
        Id add(DurationT timeout, Func func, long cycle = 1) //cycle=-1 to infinity
		{
            return add(std::chrono::duration_cast<std::chrono::system_clock::duration>(timeout), func, cycle);
		}

        Id add(std::chrono::system_clock::duration timeout, Func func, long cycle = 1); //cycle=-1 to infinity

        Id add(std::chrono::system_clock::time_point timestamp, Func func);

        Id add(std::chrono::system_clock::time_point timestamp, std::chrono::system_clock::duration period, Func func, long cycle);

		void cancel(Id id);

	private:
		void execute() final override;
	};

}

#endif // SAS_CORE__TIMELINETHREAD_H
