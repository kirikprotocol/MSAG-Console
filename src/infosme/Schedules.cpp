
#include "Schedules.h"

namespace smsc { namespace infosme 
{
    /* Advanced
    
    time_t  startDate;  // if -1 not defined, YYYY.MM.dd
    time_t  endDate;    // if -1 not defined, YYYY.MM.dd
    bool    repeat;     // default false
    int     everyNSec;  // if -1 not defined. In seconds (minutes, hours), 
    time_t  endTime;    // if -1 not defined. only HH:mm:ss
    
    */

time_t OnceSchedule::calulateNextTime()
{
    // time_t  startTime;  // only HH:mm:ss
    // time_t  startDate;  // only YYYY.MM.dd
    // Has no startTime & endDate in advanced

    if (startDate <= 0 || startTime < 0) return -1;

    time_t ct = time(NULL);
    time_t st = startDate + startTime;
    if (ct <= st) return st;
    
    if (!advanced.repeat || advanced.everyNSec <= 0) return -1;
    
    bool   ets = advanced.endTime < 0;
    time_t et = startDate+((ets) ? 0:advanced.endTime);
    
    while (st < ct && (ets ? (st < et):true)) st += advanced.everyNSec;
    return (ets ? (st < et):true) ? st:-1;
}
time_t DailySchedule::calulateNextTime()
{
    //time_t  startTime;  // only HH:mm:ss
    //int     everyNDays;
    
    if (advanced.startDate <= 0 || startTime < 0) return -1;

    time_t ct = time(NULL);
    time_t st = advanced.startDate + startTime;
    time_t deadLine = ((advanced.endDate >= 0) ? advanced.endDate:0) + 
                      ((advanced.endTime >= 0) ? advanced.endTime:0);
    if (ct >= deadLine) return -1;
    if (ct <= st && st <= deadLine) return st;
    
    if (everyNDays > 0)
    {
        while (st < ct && st < deadLine) {
            st += 86400*everyNDays;
        }
        if (st >= ct && st <= deadLine) return st;
        // use repeat here !!!
    }
    return -1;
}
time_t WeeklySchedule::calulateNextTime()
{
    return -1;
}
time_t MonthlySchedule::calulateNextTime()
{
    return -1;
}

}}
