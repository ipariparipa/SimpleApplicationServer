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

#include "include/sasCore/controlledthread.h"

#include <mutex>

namespace SAS {

	struct ControlledThread_priv
	{
		Notifier suspender_not;
		Notifier reserver_not;
	};


	ControlledThread::ControlledThread() : Thread(), priv(new ControlledThread_priv)
	{ }

	ControlledThread::~ControlledThread()
	{
		delete priv;
	}

	void ControlledThread::suspend()
	{
		if(status() == Status::NotRunning)
		{
			priv->suspender_not.wait();
			start();
		}
		else
			priv->suspender_not.wait();
	}

	void ControlledThread::resume()
	{
		priv->reserver_not.notify();
		priv->suspender_not.notify();
	}

	bool ControlledThread::reserveSuspended()
	{
		if (!priv->reserver_not.wait(0))
			return false;
		return true;
	}

	void ControlledThread::begun()
	{ }

	void ControlledThread::ended()
	{ }

	bool ControlledThread::enterContolledSection()
	{
		priv->reserver_not.notify();
		priv->suspender_not.wait();
		priv->suspender_not.notify();
		priv->reserver_not.wait();
		return true;
	}

}
