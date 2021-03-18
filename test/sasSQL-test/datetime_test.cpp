
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

void DateTime_Test::precisions()
{
    {
        SAS::SQLDateTime dt(time_t(1000), 123456, 6);
        CPPUNIT_ASSERT_EQUAL(123u, dt.milliseconds());
        CPPUNIT_ASSERT_EQUAL(123456u, dt.microseconds());
        CPPUNIT_ASSERT_EQUAL(123456000u, dt.nanoseconds());

        CPPUNIT_ASSERT_EQUAL(123456u, dt.fraction());
        CPPUNIT_ASSERT_EQUAL(6, dt.precision());
    }

    {
        auto now = std::chrono::system_clock::now();
        auto tm_time = SAS::SQLDateTime(now).to_tm();

        SAS::SQLDateTime dt(&tm_time, 1234567893, 10);
        CPPUNIT_ASSERT_EQUAL(1234567893u, dt.fraction());
        CPPUNIT_ASSERT_EQUAL(10, dt.precision());
    }

    {
        SAS::SQLDateTime dt(std::chrono::system_clock::time_point(std::chrono::system_clock::duration(1123456789)), 6);
        CPPUNIT_ASSERT_EQUAL(1970u, dt.years());

        CPPUNIT_ASSERT_EQUAL(123456u, dt.fraction());
        CPPUNIT_ASSERT_EQUAL(6, dt.precision());

        SAS::SQLDateTime dt2(std::chrono::system_clock::time_point(std::chrono::system_clock::duration(112345678901)), 8);
        CPPUNIT_ASSERT(dt < dt2);
    }

    {
        auto now = std::chrono::system_clock::now();
        SAS::SQLDateTime dt(now, 100);

        CPPUNIT_ASSERT(now == dt.toTimePoint());
    }

}
