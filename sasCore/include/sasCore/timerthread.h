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

#ifndef INCLUDE_SASCORE_TIMERTHREAD_H_
#define INCLUDE_SASCORE_TIMERTHREAD_H_

#include "thread.h"

namespace SAS {

	struct TimerThread_priv;
	class SAS_CORE__CLASS TimerThread : public Thread
	{
		SAS_COPY_PROTECTOR(TimerThread)
	public:
		TimerThread();
		virtual ~TimerThread();

		bool start(long milliseconds);
		virtual void stop() override;

		void setInterval(long milliseconds);
		long interval() const;

	protected:
		virtual void begun() override;
		virtual void ended() override;
		virtual void execute() final;
		virtual void shot() = 0;

	private:
		TimerThread_priv * priv;
	};

}

#endif /* INCLUDE_SASCORE_TIMERTHREAD_H_ */
