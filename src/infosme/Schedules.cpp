
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

Schedule* Schedule::create(ConfigView* config, std::string id)
{
    Schedule* schedule = 0;

    const char* scheduleType = config->getString("scheduleType");
    if (!scheduleType || scheduleType[0] == '\0')
        throw ConfigException("Schedule '%s' type empty or wasn't specified", 
                              id.c_str());

    if (strcmp(scheduleType, SCHEDULE_TYPE_ONCE) == 0) {
        schedule = new OnceSchedule(id);
    }
    else if (strcmp(scheduleType, SCHEDULE_TYPE_DAILY) == 0) {
        schedule = new DailySchedule(id);
    }
    else if (strcmp(scheduleType, SCHEDULE_TYPE_WEEKLY) == 0) {
        schedule = new WeeklySchedule(id);
    }
    else if (strcmp(scheduleType, SCHEDULE_TYPE_MONTHLY) == 0) {
        schedule = new MonthlySchedule(id);
    }
    else throw ConfigException("Type '%s' for schedule '%s' is unknown", 
                               scheduleType, id.c_str());

    if (schedule) schedule->init(config);
    
    return schedule;
}

void OnceSchedule::init(ConfigView* config)
{
}
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

void DailySchedule::init(ConfigView* config)
{
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

void WeeklySchedule::init(ConfigView* config)
{
}
time_t WeeklySchedule::calulateNextTime()
{
    return -1;
}

void MonthlySchedule::init(ConfigView* config)
{
}
time_t MonthlySchedule::calulateNextTime()
{
    return -1;
}

}}
