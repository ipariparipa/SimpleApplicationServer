
#include "datetime_test.h"

#include <cppunit/config/SourcePrefix.h>

#include <iostream>

#include <sasSQL/sqldatetime.h>

CPPUNIT_TEST_SUITE_REGISTRATION(DateTime_Test);

void DateTime_Test::setUp()
{
}

void DateTime_Test::tearDown()
{
}

void DateTime_Test::ctor_chrono()
{
    auto now = std::chrono::system_clock::now();

    SAS::SQLDateTime dt(now, 5);
    auto d = std::chrono::duration_cast<std::chrono::duration<int64_t, std::ratio<1, 100000>>>(now.time_since_epoch());
    CPPUNIT_ASSERT_EQUAL(d.count(), (std::chrono::duration_cast<std::chrono::duration<int64_t, std::ratio<1, 100000>>>(dt.toTimePoint().time_since_epoch())).count());
    CPPUNIT_ASSERT_EQUAL(d.count(), (std::chrono::duration_cast<std::chrono::duration<int64_t, std::ratio<1, 100000>>>(SAS::SQLDateTime(dt.toString()).toTimePoint().time_since_epoch())).count());
}
