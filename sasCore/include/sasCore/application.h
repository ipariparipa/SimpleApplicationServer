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

#ifndef INCLUDE_SASCORE_APPLICATION_H_
#define INCLUDE_SASCORE_APPLICATION_H_

#include "defines.h"
#include "errorcollector.h"
#include "logging.h"

#include <vector>
#include <mutex>

namespace SAS
{

class ConfigReader;
class InterfaceManager;
class ObjectRegistry;
class ThreadPool;

struct Application_priv;
class SAS_CORE__CLASS Application
{
    SAS_COPY_PROTECTOR(Application)
public:
	Application();
	Application(int argc, char **argv);
	virtual ~Application();

	int argc() const;
	char ** argv() const;

	ObjectRegistry * objectRegistry() const;

	virtual inline std::string version() const { return std::string(); }

	virtual bool init(ErrorCollector & ec);
	virtual void deinit();

	virtual Logging::LoggerPtr logger();

    virtual ThreadPool * threadPool();

	virtual inline InterfaceManager * interfaceManager() { return nullptr; };
	virtual ConfigReader * configReader() = 0;

    template<typename RetT>
    bool callIfEnabled(RetT & ret, std::function<RetT()> func)
    {
        std::unique_lock<Application> __locker(*this);
        if(isEnabled())
        {
            ret = func();
            return true;
        }

        return false;
    }

    template<typename RetT>
    RetT callIfEnabled(std::function<RetT()> func, const RetT & defaultRet)
    {
        std::unique_lock<Application> __locker(*this);
        if(isEnabled())
            return func();

        return defaultRet;
    }

    bool callIfEnabled(std::function<void()> func)
    {
        std::unique_lock<Application> __locker(*this);
        if(isEnabled())
        {
            func();
            return true;
        }

        return false;
    }

private:
    friend class std::unique_lock<Application>;

    void lock();
    void unlock();
    bool isEnabled();

	Application_priv * priv;
};

}

#endif /* INCLUDE_SASCORE_APPLICATION_H_ */
