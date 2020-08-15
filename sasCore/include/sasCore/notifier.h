#ifndef SAS_CORE__NOTIFIER_H
#define SAS_CORE__NOTIFIER_H

#include "defines.h"
#include <chrono>

class SAS_CORE__CLASS Notifier
{
    SAS_COPY_PROTECTOR(Notifier)
    struct Priv;
    Priv * p;
public:
    Notifier();
    ~Notifier();

    void notify();

    void notifyAll();

    void wait();

    inline bool wait(long milliseconds)
    { return wait(std::chrono::milliseconds(milliseconds)); }

    bool wait(std::chrono::milliseconds timeout_ms);

    bool wait(std::chrono::time_point<std::chrono::system_clock> time);
#if SAS_OS != SAS_OS_WINDOWS
    bool wait(std::chrono::time_point<std::chrono::steady_clock> time);
#endif
    bool tryWait();
};

#endif // SAS_CORE__NOTIFIER_H
