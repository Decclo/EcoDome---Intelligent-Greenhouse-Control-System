#include <time.h>
#include <stdio.h>

int Get_TZ_delta(const struct tm *tmptr) 
{
    // Make local copy
    struct tm tm = *tmptr;
    time_t t = mktime(&tm);
    struct tm utc_tm = *gmtime(&t);
    time_t t2 = mktime(&utc_tm);
    return (int) difftime(t, t2);
}

time_t UniversalTimeStamp_to_time_t(const char *ts) 
{
    struct tm tm = { 0 };
    double seconds;
    time_t timer;
    time(&timer);  /* get current time; same as: timer = time(NULL)  */
    // Use a sentinel to catch extra garbage
    char sentinel;
    if (sscanf(ts, "Valid at %d-%2d-%2dT%2d:%2d:%2d%c", &tm.tm_year, &tm.tm_mon,
    &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec, &sentinel) != 6) 
    {
        return -1;
    }
    // struct tm uses offset from 1900 and January is month 0
    tm.tm_year -= 1900;
    tm.tm_mon--;
    // Convert tm from UCT to local standard time
    tm.tm_isdst = 0;
    tm.tm_sec += Get_TZ_delta(&tm);

    time_t t = mktime(&tm); // mktime() assumes tm is local

    // test code
    {
        //struct tm *local_time = localtime(&timer);
        time_t tnow = timegm(localtime(&t));
        double seconds2 = difftime(timer,mktime(&tm));
        printf("Unix  %lld\n", (long long) time);
        printf ("%.f\n", seconds);
        printf ("%.f\n", seconds2);


        printf("UTC  `%s`\n", ts);
        char buf[100];
        strftime(buf, sizeof buf, "%Y-%m-%dT%H:%M:%S %Z", &tm);
        printf("Local %s\n", buf);
        struct tm utc_tm = *gmtime(&t);
        strftime(buf, sizeof buf, "%Y-%m-%dT%H:%M:%S %Z", &utc_tm);
        printf("Local %s\n", buf);
        printf("Unix  %lld\n\n", (long long) mktime(&tm));
    }

  return t;
}

int main(void) {
  UniversalTimeStamp_to_time_t("Valid at 2018-05-18T19:22:00");
  return 0;
}