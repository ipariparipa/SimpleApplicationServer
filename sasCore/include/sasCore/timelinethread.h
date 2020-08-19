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


        TimelineThread(ThreadPool * pool);
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
