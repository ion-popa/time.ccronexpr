/* 
 * File:   CronExprParser_test.cpp
 * Author: alex
 *
 * Created on February 24, 2015, 9:36 AM
 */
#ifdef CRON_TEST
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "ccronexpr.h"

#define MAX_SECONDS 60
#define CRON_MAX_MINUTES 60
#define CRON_MAX_HOURS 24
#define CRON_MAX_DAYS_OF_WEEK 8
#define CRON_MAX_DAYS_OF_MONTH 32
#define CRON_MAX_MONTHS 12

#define INVALID_INSTANT ((time_t) -1)

#define DATE_FORMAT "%Y-%m-%d_%H:%M:%S"

#ifndef ANDROID
    #ifndef _WIN32
time_t timegm(struct tm* __tp);
    #else /* _WIN32 */
static time_t timegm(struct tm* tm) {
    return _mkgmtime(tm);
}
    #endif /* _WIN32 */
#else /* ANDROID */
static time_t timegm(struct tm * const t) {
    /* time_t is signed on Android. */
    static const time_t kTimeMax = ~(1L << (sizeof (time_t) * CHAR_BIT - 1));
    static const time_t kTimeMin = (1L << (sizeof (time_t) * CHAR_BIT - 1));
    time64_t result = timegm64(t);
    if (result < kTimeMin || result > kTimeMax)
        return -1;
    return result;
}
#endif

static int crons_equal(cron_expr* cr1, cron_expr* cr2) {
    int i;
    for (i = 0; i < MAX_SECONDS; i++) {
        if (cr1->seconds[i] != cr2->seconds[i]) {
            return 0;
        }
    }
    for (i = 0; i < CRON_MAX_MINUTES; i++) {
        if (cr1->minutes[i] != cr2->minutes[i]) {
            return 0;
        }
    }
    for (i = 0; i < CRON_MAX_HOURS; i++) {
        if (cr1->hours[i] != cr2->hours[i]) {
            return 0;
        }
    }
    for (i = 0; i < CRON_MAX_DAYS_OF_WEEK; i++) {
        if (cr1->days_of_week[i] != cr2->days_of_week[i]) {
            return 0;
        }
    }
    for (i = 0; i < CRON_MAX_DAYS_OF_MONTH; i++) {
        if (cr1->days_of_month[i] != cr2->days_of_month[i]) {
            return 0;
        }
    }
    for (i = 0; i < CRON_MAX_MONTHS; i++) {
        if (cr1->months[i] != cr2->months[i]) {
            return 0;
        }
    }
    return 1;
}

int one_dec_num(const char ch) {
    switch(ch) {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        default: return -1;
    }
}

int two_dec_num(const char* first) {
    return one_dec_num(first[0])*10 + one_dec_num(first[1]);
}

/* strptime is not available in msvc */
/* 2012-07-01_09:53:50 */
/* 0123456789012345678 */
struct tm* poors_mans_strptime(const char* str) {
    struct tm* cal = (struct tm*) malloc(sizeof (struct tm));
    switch(str[3]) {
        case '7': cal->tm_year = 107; break;
        case '8': cal->tm_year = 108; break;
        case '9': cal->tm_year = 109; break;
        case '0': cal->tm_year = 110; break;
        case '1': cal->tm_year = 111; break;
        case '2': cal->tm_year = 112; break;
    }
    cal->tm_mon = two_dec_num(str + 5) - 1;
    cal->tm_mday = two_dec_num(str + 8);
    cal->tm_wday = 0;
    cal->tm_yday = 0;
    cal->tm_hour = two_dec_num(str + 11);
    cal->tm_min = two_dec_num(str + 14);
    cal->tm_sec = two_dec_num(str + 17);
    return cal;
}

void check_next(const char* pattern, const char* initial, const char* expected) {
    const char* err = NULL;
    cron_expr* parsed = cron_parse_expr(pattern, &err);
    struct tm* calinit = poors_mans_strptime(initial);
    time_t dateinit = timegm(calinit);
    assert(-1 != dateinit);
    time_t datenext = cron_next(parsed, dateinit);
    struct tm* calnext = gmtime(&datenext);
    assert(calnext);
    char* buffer = (char*) malloc(21);
    memset(buffer, 0, 21);
    strftime(buffer, 20, DATE_FORMAT, calnext);
    if(0 != strcmp(expected, buffer)) {
        puts(expected);
        puts(buffer);
        assert(0);
    }
    free(buffer);
    free(calinit);
    cron_expr_free(parsed);
}

void check_same(const char* expr1, const char* expr2) {
    cron_expr* parsed1 = cron_parse_expr(expr1, NULL);
    cron_expr* parsed2 = cron_parse_expr(expr2, NULL);
    assert(crons_equal(parsed1, parsed2));
    cron_expr_free(parsed1);
    cron_expr_free(parsed2);
}

void check_calc_invalid() {
    cron_expr* parsed = cron_parse_expr("0 0 0 31 6 *", NULL);
    struct tm * calinit = poors_mans_strptime("2012-07-01_09:53:50");
    time_t dateinit = timegm(calinit);
    time_t res = cron_next(parsed, dateinit);
    assert(INVALID_INSTANT == res);
    free(calinit);
    cron_expr_free(parsed);
}

void check_expr_invalid(const char* expr) {
    const char* err = NULL;
    cron_parse_expr(expr, &err);
    assert(err);
}

void test_expr() {
    check_next("*/15 * 1-4 * * *",  "2012-07-01_09:53:50", "2012-07-02_01:00:00");
    check_next("*/15 * 1-4 * * *",  "2012-07-01_09:53:00", "2012-07-02_01:00:00");
    check_next("0 */2 1-4 * * *",   "2012-07-01_09:00:00", "2012-07-02_01:00:00");
    check_next("* * * * * *",       "2012-07-01_09:00:00", "2012-07-01_09:00:01");
    check_next("* * * * * *",       "2012-12-01_09:00:58", "2012-12-01_09:00:59");
    check_next("10 * * * * *",      "2012-12-01_09:42:09", "2012-12-01_09:42:10");
    check_next("11 * * * * *",      "2012-12-01_09:42:10", "2012-12-01_09:42:11");
    check_next("10 * * * * *",      "2012-12-01_09:42:10", "2012-12-01_09:43:10");
    check_next("10-15 * * * * *",   "2012-12-01_09:42:09", "2012-12-01_09:42:10");
    check_next("10-15 * * * * *",   "2012-12-01_21:42:14", "2012-12-01_21:42:15");
    check_next("0 * * * * *",       "2012-12-01_21:10:42", "2012-12-01_21:11:00");
    check_next("0 * * * * *",       "2012-12-01_21:11:00", "2012-12-01_21:12:00");
    check_next("0 11 * * * *",      "2012-12-01_21:10:42", "2012-12-01_21:11:00");
    check_next("0 10 * * * *",      "2012-12-01_21:11:00", "2012-12-01_22:10:00");
    check_next("0 0 * * * *",       "2012-09-30_11:01:00", "2012-09-30_12:00:00");
    check_next("0 0 * * * *",       "2012-09-30_12:00:00", "2012-09-30_13:00:00");
    check_next("0 0 * * * *",       "2012-09-10_23:01:00", "2012-09-11_00:00:00");
    check_next("0 0 * * * *",       "2012-09-11_00:00:00", "2012-09-11_01:00:00");
    check_next("0 0 0 * * *",       "2012-09-01_14:42:43", "2012-09-02_00:00:00");
    check_next("0 0 0 * * *",       "2012-09-02_00:00:00", "2012-09-03_00:00:00");
    check_next("* * * 10 * *",      "2012-10-09_15:12:42", "2012-10-10_00:00:00");
    check_next("* * * 10 * *",      "2012-10-11_15:12:42", "2012-11-10_00:00:00");
    check_next("0 0 0 * * *",       "2012-09-30_15:12:42", "2012-10-01_00:00:00");
    check_next("0 0 0 * * *",       "2012-10-01_00:00:00", "2012-10-02_00:00:00");
    check_next("0 0 0 * * *",       "2012-08-30_15:12:42", "2012-08-31_00:00:00");
    check_next("0 0 0 * * *",       "2012-08-31_00:00:00", "2012-09-01_00:00:00");
    check_next("0 0 0 * * *",       "2012-10-30_15:12:42", "2012-10-31_00:00:00");
    check_next("0 0 0 * * *",       "2012-10-31_00:00:00", "2012-11-01_00:00:00");
    check_next("0 0 0 1 * *",       "2012-10-30_15:12:42", "2012-11-01_00:00:00");
    check_next("0 0 0 1 * *",       "2012-11-01_00:00:00", "2012-12-01_00:00:00");
    check_next("0 0 0 1 * *",       "2010-12-31_15:12:42", "2011-01-01_00:00:00");
    check_next("0 0 0 1 * *",       "2011-01-01_00:00:00", "2011-02-01_00:00:00");
    check_next("0 0 0 31 * *",      "2011-10-30_15:12:42", "2011-10-31_00:00:00");
    check_next("0 0 0 1 * *",       "2011-10-30_15:12:42", "2011-11-01_00:00:00");
    check_next("* * * * * 2",       "2010-10-25_15:12:42", "2010-10-26_00:00:00");
    check_next("* * * * * 2",       "2010-10-20_15:12:42", "2010-10-26_00:00:00");
    check_next("* * * * * 2",       "2010-10-27_15:12:42", "2010-11-02_00:00:00");
    check_next("55 5 * * * *",      "2010-10-27_15:04:54", "2010-10-27_15:05:55");
    check_next("55 5 * * * *",      "2010-10-27_15:05:55", "2010-10-27_16:05:55");
    check_next("55 * 10 * * *",     "2010-10-27_09:04:54", "2010-10-27_10:00:55");
    check_next("55 * 10 * * *",     "2010-10-27_10:00:55", "2010-10-27_10:01:55");
    check_next("* 5 10 * * *",      "2010-10-27_09:04:55", "2010-10-27_10:05:00");
    check_next("* 5 10 * * *",      "2010-10-27_10:05:00", "2010-10-27_10:05:01");
    check_next("55 * * 3 * *",      "2010-10-02_10:05:54", "2010-10-03_00:00:55");
    check_next("55 * * 3 * *",      "2010-10-03_00:00:55", "2010-10-03_00:01:55");
    check_next("* * * 3 11 *",      "2010-10-02_14:42:55", "2010-11-03_00:00:00");
    check_next("* * * 3 11 *",      "2010-11-03_00:00:00", "2010-11-03_00:00:01");
    check_next("0 0 0 29 2 *",      "2007-02-10_14:42:55", "2008-02-29_00:00:00");
    check_next("0 0 0 29 2 *",      "2008-02-29_00:00:00", "2012-02-29_00:00:00");
    check_next("0 0 7 ? * MON-FRI", "2009-09-26_00:42:55", "2009-09-28_07:00:00");
    check_next("0 0 7 ? * MON-FRI", "2009-09-28_07:00:00", "2009-09-29_07:00:00");
    check_next("0 30 23 30 1/3 ?",  "2010-12-30_00:00:00", "2011-01-30_23:30:00");
    check_next("0 30 23 30 1/3 ?",  "2011-01-30_23:30:00", "2011-04-30_23:30:00");
    check_next("0 30 23 30 1/3 ?",  "2011-04-30_23:30:00", "2011-07-30_23:30:00");    
}

void test_parse() {
    check_same("* * * 2 * *", "* * * 2 * ?");
    check_same("57,59 * * * * *", "57/2 * * * * *");
    check_same("1,3,5 * * * * *", "1-6/2 * * * * *");
    check_same("* * 4,8,12,16,20 * * *", "* * 4/4 * * *");
    check_same("* * * * * 0-6", "* * * * * TUE,WED,THU,FRI,SAT,SUN,MON");
    check_same("* * * * * 0", "* * * * * SUN");
    check_same("* * * * * 0", "* * * * * 7");
    check_same("* * * * 1-12 *", "* * * * FEB,JAN,MAR,APR,MAY,JUN,JUL,AUG,SEP,OCT,NOV,DEC *");
    check_same("* * * * 2 *", "* * * * Feb *");
    check_same("*  *  * *  1 *", "* * * * 1 *");
    
    check_expr_invalid("77 * * * * *");
    check_expr_invalid("44-77 * * * * *");
    check_expr_invalid("* 77 * * * *");
    check_expr_invalid("* 44-77 * * * *");
    check_expr_invalid("* * 27 * * *");
    check_expr_invalid("* * 23-28 * * *");
    check_expr_invalid("* * * 45 * *");
    check_expr_invalid("* * * 28-45 * *");
    check_expr_invalid("0 0 0 25 13 ?");
    check_expr_invalid("0 0 0 25 0 ?");
    check_expr_invalid("0 0 0 32 12 ?");
    check_expr_invalid("* * * * 11-13 *");
}

int main() {
    test_expr();
    test_parse();
    check_calc_invalid();

    return 0;
}
#endif

