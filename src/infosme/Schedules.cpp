
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
    if (deadLine > 0 && ct >= deadLine) return -1;
    if (ct <= st) return ((deadLine <= 0) ? st:((st < deadLine) ? st:-1));
    
    if (everyNDays <= 0) return -1;
    int interval = 86400*everyNDays;
    st += ((ct-st)/interval+1)*interval;
    return ((deadLine <= 0) ? st:((st < deadLine) ? st:-1));
}

static const char*  constFullEngMonthesNames[12] = {
    "January", "February", "March", "April",
    "May", "June", "July", "August", "September",
    "October", "November", "December"
};
static const char*  constShortEngMonthesNames[12] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", 
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
static const char*  constFullEngWeekDays[7] = {
    "Monday", "Tuesday", "Wednesday", "Thursday", 
    "Friday", "Saturday", "Sunday"
};
static const char*  constShortEngWeekDays[7] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};
static const char*  constFullEngWeekDayN[5] = {
    "first", "second", "third", "fourth", "last"
};

int scanWeekDay(const char* str)
{
    if (!str || str[0] == '\0') return -1;
    for (int i=0; i<7; i++)
        if (strcmp(constFullEngWeekDays[i], str) == 0 ||
            strcmp(constShortEngWeekDays[i], str) == 0) return i;
    return -1;    
}
int scanMonthName(const char* str)
{
    if (!str || str[0] == '\0') return -1;
    for (int i=0; i<12; i++)
        if (strcmp(constFullEngMonthesNames[i], str) == 0 ||
            strcmp(constShortEngMonthesNames[i], str) == 0) return i;
    return -1;    
}
int scanWeekDayN(const char* str)
{
    if (!str || str[0] == '\0') return -1;
    for (int i=0; i<7; i++)
        if (strcmp(constFullEngWeekDayN[i], str) == 0) return i;
    return -1;    
}

bool WeekDaysParser::initWeekDays(std::string weekDays)
{
    // ',' separated list Mon, Tue, ...
    memset(&weekDaysSet, 0, sizeof(weekDaysSet));

    const char* str = weekDays.c_str();
    if (!str || str[0] == '\0') return false;
    
    std::string weekDay = "";
    do
    {
        if (*str == ',' || *str == '\0') {
            int day = scanWeekDay(weekDay.c_str());
            if (day < 0 || day > 6) return false;
            weekDaysSet[day] = day;
            weekDay = "";
        } 
        else weekDay += *str;
    } 
    while (*str++);
    return true;
}
void WeeklySchedule::init(ConfigView* config)
{
    Schedule::init(config, true);

    everyNWeeks = config->getInt("everyNWeeks");
    if (everyNWeeks <= 0)
        throw ConfigException("Invalid everyNWeeks parameter, should be positive.");
    if (!initWeekDays(config->getString("weekDays")))
        throw ConfigException("Invalid weekDay parameter, should be "
                              "',' separated list of week days.");
}
time_t WeeklySchedule::calulateNextTime()
{
    if (startDate <= 0 || startTime < 0 || everyNWeeks <= 0) return -1;
    
    time_t ct = time(NULL);
    time_t deadLine = ((endDate >= 0) ? endDate:0) + 
                      ((endTime >= 0) ? endTime:0);
    if (deadLine > 0 && ct >= deadLine) return -1;
    time_t st = startDate;
    
    tm dt; localtime_r(&st, &dt);
    dt.tm_mday -= ((dt.tm_wday == 0) ? 6:(dt.tm_wday-1));
    st = mktime(&dt); // Понедельник стартовой недели.

    int interval = 86400*7*everyNWeeks;
    st += ((ct-st)/interval)*interval+startTime; // понедельник очередной недели + startTime
    
    while (st < ct)
    {
        if (deadLine > 0 && st >= deadLine) return -1;
        for (int i=0; i<7; i++) {
            if (st < ct) st += weekDaysSet[i]*86400;
            else return ((deadLine <= 0) ? st:((st < deadLine) ? st:-1));
        }
        st += interval;
    }
    
    return ((deadLine <= 0) ? st:((st < deadLine) ? st:-1));
}

bool MonthesNamesParser::initWeekDayN(std::string weekDayNs)
{
    // ',' separated list: first, second, third, fourth, last.
    memset(&weekDayNSet, 0, sizeof(weekDayNSet));

    const char* str = weekDayNs.c_str();
    if (!str || str[0] == '\0') return false;
    
    std::string weekDayN = "";
    do
    {
        if (*str == ',' || *str == '\0') {
            int n = scanWeekDayN(weekDayN.c_str());
            if (n < 0 || n > 4) return false;
            weekDayNSet[n] = n;
            weekDayN = "";
        } 
        else weekDayN += *str;
    } 
    while (*str++);
    return true;

}
bool MonthesNamesParser::initMonthesNames(std::string monthesNames)
{
    // ',' separated list Jan, Feb, ...
    memset(&monthesNamesSet, 0, sizeof(monthesNamesSet));

    const char* str = monthesNames.c_str();
    if (!str || str[0] == '\0') return false;
    
    std::string monthName = "";
    do
    {
        if (*str == ',' || *str == '\0') {
            int month = scanMonthName(monthName.c_str());
            if (month < 0 || month > 11) return false;
            monthesNamesSet[month] = true;
            monthName = "";
        } 
        else monthName += *str;
    } 
    while (*str++);
    return true;
}

void MonthlySchedule::init(ConfigView* config)
{
    Schedule::init(config, true);
    
    dayOfMonth = -1;
    try { dayOfMonth = config->getInt("dayOfMonth"); } catch (...) {};
    if (dayOfMonth == 0 || dayOfMonth > 31)
        throw ConfigException("Invalid dayOfMonth parameter, "
                              "should be in interval [1, 31] or skipped when using weeks.");
    if (dayOfMonth == -1)
    {
        if (!initWeekDayN(config->getString("weekDayN")))
            throw ConfigException("Invalid weekDayN parameter, should be "
                                  "',' separated list of week numbers: "
                                  "first, second, third, fourth, last.");
        if (!initWeekDays(config->getString("weekDays")))
            throw ConfigException("Invalid weekDay parameter, should be "
                                  "',' separated list of week days.");
    }
    if (!initMonthesNames(config->getString("monthes")))
        throw ConfigException("Invalid monthes parameter, should be "
                              "',' separated list of monthes names.");
}
time_t MonthlySchedule::calulateNextTime()
{
    if (startDate <= 0 || startTime < 0 || 
        dayOfMonth == 0 || dayOfMonth > 31) return -1;
    
    time_t ct = time(NULL);
    time_t deadLine = ((endDate >= 0) ? endDate:0) + 
                      ((endTime >= 0) ? endTime:0);
    time_t st = startDate + startTime;
    if (deadLine > 0 && (ct >= deadLine || st >= deadLine)) return -1;
    
    while (1)
    {
        tm dt; localtime_r(&st, &dt);
        if (dt.tm_mon < 0 || dt.tm_mon >= 12) return -1;
        dt.tm_sec = 0; dt.tm_min = 0; dt.tm_hour = 0;
        
        if (dayOfMonth > 0)
        {
            dt.tm_mday = dayOfMonth; 
            time_t at = mktime(&dt) + startTime;
            if (at > ct && at >= st) 
                return ((deadLine <= 0) ? at:((at < deadLine) ? at:-1))
        }
        else
        {
            // TODO: calculate by weekDays number

        }

        // increase tm_mon by monthesNames
        int start_mon = dt.tm_mon;
        while (++dt.tm_mon < start_mon+12) {
            int index = dt.tm_mon-start_mon;
            if (index>=0 && index<12 && monthesNamesSet[index]) break;
        }
        if (dt.tm_mon >= start_mon+12) return -1; // no monthes found !!!
    }
    
    return ((deadLine <= 0) ? st:((st < deadLine) ? st:-1));
}

void IntervalSchedule::init(ConfigView* config)
{
    Schedule::init(config, true);
    
    intervalTime = parseTime(config->getString("intervalTime"));
    if (intervalTime <= 0) 
        throw ConfigException("Invalid time interval. Format is HH:mm:ss");
}
time_t IntervalSchedule::calulateNextTime()
{
    if (startDate <= 0 || startTime < 0) return -1;

    time_t ct = time(NULL);
    time_t st = startDate + startTime;
    time_t deadLine = ((endDate >= 0) ? endDate:0) + 
                      ((endTime >= 0) ? endTime:0);
    if (deadLine > 0 && ct >= deadLine) return -1;
    if (ct <= st) return ((deadLine <= 0) ? st:((st < deadLine) ? st:-1));
    if (intervalTime <= 0) return -1;
    
    st += ((ct-st)/intervalTime + 1)*intervalTime;
    return ((deadLine <= 0) ? st:((st < deadLine) ? st:-1));
}

}}
