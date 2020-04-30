Cron expression parsing in ANSI C
=================================

A fork of [time.ccronexp from mkn](https://github.com/mkn/time.ccronexpr). I've simply added a #define to allow this to run on ESP8266 and VR boards.  I've also added the files to allow this library to be pulled in with platformIO. 

Given a cron expression and a date, you can get the next date which satisfies the cron expression.

Supports cron expressions with `seconds` field. Based on implementation of [CronSequenceGenerator](https://github.com/spring-projects/spring-framework/blob/babbf6e8710ab937cd05ece20270f51490299270/spring-context/src/main/java/org/springframework/scheduling/support/CronSequenceGenerator.java) from Spring Framework.

Compiles and should work on Linux (GCC/Clang), Mac OS (Clang), Windows (MSVC), Android NDK, iOS, Raspberri Pi and possibly on other platforms with `time.h` support.

Supports compilation in C (89) and in C++ modes.

Usage example
-------------

    #include "ccronexpr.h"

    const char* err = NULL;
    cron_expr* expr = cron_parse_expr("0 */2 1-4 * * *", &err);
    if (err) ... /* invalid expression */
    time_t cur = time(NULL);
    time_t next = cron_next(expr, cur);
    ...
    cron_expr_free(expr);


Compilation and tests run examples
----------------------------------

     gcc ccronexpr.c ccronexpr_test.c -I. -Wall -Wextra -std=c89 && ./a.out
     g++ ccronexpr.c ccronexpr_test.c -I. -Wall -Wextra -std=c++11 && ./a.out

     clang ccronexpr.c ccronexpr_test.c -I. -Wall -Wextra -std=c89 && ./a.out
     clang++ ccronexpr.c ccronexpr_test.c -I. -Wall -Wextra -std=c++11 && ./a.out

     cl ccronexpr.c ccronexpr_test.c /W4 /D_CRT_SECURE_NO_WARNINGS & ccronexpr.exe

Examples of supported expressions
---------------------------------

Expression, input date, next date:

    "*/15 * 1-4 * * *",  "2012-07-01_09:53:50", "2012-07-02_01:00:00"
    "0 */2 1-4 * * *",   "2012-07-01_09:00:00", "2012-07-02_01:00:00"
    "0 0 7 ? * MON-FRI", "2009-09-26_00:42:55", "2009-09-28_07:00:00"
    "0 30 23 30 1/3 ?",  "2011-04-30_23:30:00", "2011-07-30_23:30:00"

See more examples in tests.

Timezones
---------

This implementation does not support explicit timezones handling. By default all dates are
processed as UTC (GMT) dates without timezone infomation. 

To use local dates (current system timezone) instead of GMT compile with `-DCRON_USE_LOCAL_TIME`.

License information
-------------------

This project is released under the [Apache License 2.0](http://www.apache.org/licenses/LICENSE-2.0)

Changelog
---------

**1.0** (2015-02-28)

 * initial public version
