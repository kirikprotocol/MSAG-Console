
#include "Schedules.h"

namespace smsc { namespace infosme 
{

Schedule* Schedule::create(ConfigView* config, std::string id)
{
    Schedule* schedule = 0;

    const char* execute = config->getString("execute");
    if (!execute || execute[0] == '\0')
        throw ConfigException("Schedule '%s' type empty or wasn't specified", 
                              id.c_str());

    if (strcmp(execute, SCHEDULE_TYPE_ONCE) == 0) {
        schedule = new OnceSchedule(id);
    }
    else if (strcmp(execute, SCHEDULE_TYPE_DAILY) == 0) {
        schedule = new DailySchedule(id);
    }
    else if (strcmp(execute, SCHEDULE_TYPE_WEEKLY) == 0) {
        schedule = new WeeklySchedule(id);
    }
    else if (strcmp(execute, SCHEDULE_TYPE_MONTHLY) == 0) {
        schedule = new MonthlySchedule(id);
    }
    else if (strcmp(execute, SCHEDULE_TYPE_INTERVAL) == 0) {
        schedule = new IntervalSchedule(id);
    }
    else throw ConfigException("Execution mode '%s' for schedule '%s' is unknown", 
                               execute, id.c_str());

    if (schedule) schedule->init(config);
    
    return schedule;
}
void Schedule::init(ConfigView* config, bool full)
{
    startTime = parseTime(config->getString("startTime")); 
    if (startTime < 0)
        throw ConfigException("Invalid startTime parameter.");
    startDate = parseDate(config->getString("startDate"));
    if (startDate <= 0)
        throw ConfigException("Invalid startDate parameter.");
    
    if (full) {
        endDate = -1; endTime = -1;
        try { endDate = parseDate(config->getString("endDate")); } catch(...) {};
        try { endTime = parseTime(config->getString("endTime")); } catch(...) {};
    }
}
void OnceSchedule::init(ConfigView* config)
{
    Schedule::init(config, false);
}
time_t OnceSchedule::calulateNextTime()
{
    if (startDate <= 0 || startTime < 0) return -1;

    time_t ct = time(NULL);
    time_t st = startDate + startTime;
    return ((ct <= st) ? st:-1);
}

void DailySchedule::init(ConfigView* config)
{
    Schedule::init(config, true);

    everyNDays = config->getInt("everyNDays");
    if (everyNDays <= 0)
        throw ConfigException("Invalid everyNDays parameter, should be positive.");
}
time_t DailySchedule::calulateNextTime()
{
    if (startDate <= 0 || startTime < 0) return -1;

    time_t ct = time(NULL);
    time_t st = startDate + startTime;
    time_t deadLine = ((endDate >= 0) ? endDate:0) + 
                      ((endTime >= 0) ? endTime:0);
    if (ct >= deadLine) return -1;
    if (ct <= st && st <= deadLine) return st;
    if (everyNDays <= 0) return -1;
    
    int interval = 86400*everyNDays;
    st += ((ct-st)/interval+1)*interval;
    return ((st <= deadLine) ? st:-1);
}

void WeeklySchedule::init(ConfigView* config)
{
    Schedule::init(config, true);

    everyNWeeks = config->getInt("everyNWeeks");
    if (everyNWeeks <= 0)
        throw ConfigException("Invalid everyNWeeks parameter, should be positive.");
    
    weekDays = config->getString("weekDays");
    // TODO: check weekDays.
    memset(&weekDaysSet, 0, sizeof(weekDaysSet));
    weekDaysSet[0] = 0;
}
time_t WeeklySchedule::calulateNextTime()
{
    if (startDate <= 0 || startTime < 0 || everyNWeeks <= 0) return -1;
    
    time_t ct = time(NULL);
    time_t deadLine = ((endDate >= 0) ? endDate:0) + 
                      ((endTime >= 0) ? endTime:0);
    if (ct >= deadLine) return -1;
    time_t st = startDate;
    
    /* TODO: 
            1) вычисляем понедельник стартовой недели (по StartDate+StartTime)
            2) вычисляем понедельник последней недели запуска.
            3) добавляем дни недели и смотрим когда они станут больше чем текущее время
            4) если перебрали все, то добавляем интервал до след. понедельника.
            5) добавляем дни недели и смотрим когда они станут больше чем текущее время
            6) проверяем что вычисленное время не больше deadLine
    */
    tm dt; localtime_r(&st, &dt);
    dt.tm_mday -= ((dt.tm_wday == 0) ? 6:(dt.tm_wday-1));
    st = mktime(&dt); // Понедельник стартовой недели.

    int interval = 86400*7*everyNWeeks;
    st += ((ct-st)/interval)*interval+startTime; // понедельник очередной недели + startTime
    
    while (st < ct && st < deadLine)
    {
        for (int i=0; i<7; i++) {
            if (st < ct) st += weekDaysSet[i]*86400;
            else return ((st <= deadLine) ? st:-1);
        }
        st += interval;
    }
    
    __require__(0); // unreachable code !!!
    return -1;
}

void MonthlySchedule::init(ConfigView* config)
{
    Schedule::init(config, true);
    
    dayOfMonth = -1;
    try { dayOfMonth = config->getInt("dayOfMonth"); } catch (...) {};
    if (dayOfMonth == -1) {
        weekN = config->getString("weekN"); // week number: first, second, third, fourth, last
        weekDays = config->getString("weekDays");
    }
    monthes = config->getString("monthes");

    // TODO: check weekDays && monthes.
}
time_t MonthlySchedule::calulateNextTime()
{
    return -1;
}

void IntervalSchedule::init(ConfigView* config)
{
    Schedule::init(config, true);
    
    intervalTime = config->getInt("intervalTime");
    if (intervalTime <= 0) 
        throw ConfigException("Invalid time interval.");
}
time_t IntervalSchedule::calulateNextTime()
{
    if (startDate <= 0 || startTime < 0) return -1;

    time_t ct = time(NULL);
    time_t st = startDate + startTime;
    time_t deadLine = ((endDate >= 0) ? endDate:0) + 
                      ((endTime >= 0) ? endTime:0);
    if (ct >= deadLine) return -1;
    if (ct <= st && st <= deadLine) return st;
    if (intervalTime <= 0) return -1;
    
    st += ((ct-st)/intervalTime + 1)*intervalTime;
    return ((st <= deadLine) ? st:-1);
}


}}
