
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
		Id add(DurationT timeout, Func func)
		{
			return add(std::chrono::system_clock::now() + timeout, func);
		}

		Id add(std::chrono::system_clock::time_point timestamp, Func func);

		void cancel(Id id);

	private:
		void execute() final override;
	};

}

#endif // SAS_CORE__TIMELINETHREAD_H
