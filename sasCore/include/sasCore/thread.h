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

#ifndef INCLUDE_SASCORE_THREAD_H_
#define INCLUDE_SASCORE_THREAD_H_

#include "basictypes.h"
#include "defines.h"

#include <thread>
#include <string>

namespace SAS
{

class ThreadPool;

typedef std::thread::id ThreadId;

struct Thread_priv;
class SAS_CORE__CLASS Thread
{
	SAS_COPY_PROTECTOR(Thread)
	friend struct Thread_priv;
public:
	enum class Status
	{
		NotRunning,
		Started,
		Running,
		Stopped
	};

    Thread(ThreadPool * pool);
	virtual ~Thread();

	ThreadId id();
    std::string name() const;
	virtual bool start();
	virtual void stop();
	void wait();

    bool inline wait(long milliseconds)
    { return wait(std::chrono::milliseconds(milliseconds)); }

    bool wait(std::chrono::milliseconds timeout);
    void terminate();
	Status status();

	static ThreadId getThreadId();

	static void sleep(long milliseconds);

protected:
	inline virtual void begun() { }
	inline virtual void ended() { }
	virtual void execute() = 0;
private:
	Thread_priv * priv;
};

}

#endif /* INCLUDE_SASCORE_THREAD_H_ */
