
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
    startDateTime = parseDateTime(config->getString("startDateTime"));
    if (startDateTime <= 0)
        throw ConfigException("Invalid startDateTime parameter.");

    if (full) {
        deadLine = -1;
        try { deadLine = parseDateTime(config->getString("endDateTime")); } catch(...) {};
    }
}

void OnceSchedule::init(ConfigView* config)
{
    Schedule::init(config, false);
}
time_t OnceSchedule::calulateNextTime()
{
    return ((startDateTime > 0 && time(NULL) < startDateTime) ? startDateTime:-1);
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
    if (startDateTime <= 0) return -1;

    time_t ct = time(NULL);
    time_t st = startDateTime;
    if (deadLine > 0 && ct >= deadLine) return -1;
    if (ct < st) return ((deadLine <= 0) ? st:((st < deadLine) ? st:-1));
    if (everyNDays <= 0) return -1;

    tm dt; localtime_r(&st, &dt);
    int intervalInSeconds = 86400*everyNDays;
    dt.tm_mday += ((ct-st)/intervalInSeconds+1)*everyNDays;
    st = mktime(&dt); 

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
    if (startDateTime <= 0) return -1;
    
    time_t ct = time(NULL);
    if (deadLine > 0 && ct >= deadLine) return -1;
    time_t st = startDateTime;
    
    tm dt; localtime_r(&st, &dt);
    dt.tm_mday -= ((dt.tm_wday == 0) ? 6:(dt.tm_wday-1));
    st = mktime(&dt); // Понедельник первой стартовой недели.

    int intervalInDays = 7*everyNWeeks;
    int intervalInSeconds = intervalInDays*86400;

    if (ct >= st) { 
        dt.tm_mday += ((ct-st)/intervalInSeconds)*intervalInDays;
        st = mktime(&dt); // Понедельник очередной стартовой недели.
    }
    
    while (st < startDateTime || st <= ct)
    {
        if (deadLine > 0 && st >= deadLine) return -1;
        for (int i=0; i<7; i++) {
            if (st <= ct) {
                dt.tm_mday += weekDaysSet[i];
                st = mktime(&dt);
            }
            else if (st > startDateTime) {
                return ((deadLine <= 0) ? st:((st < deadLine) ? st:-1));
            }
        }
        dt.tm_mday += intervalInDays;
        st = mktime(&dt); // Переходим к следующей недели и ... 
        dt.tm_mday -= ((dt.tm_wday == 0) ? 6:(dt.tm_wday-1));
        st = mktime(&dt); // Берём на ней понедельник
    }
    
    return ((deadLine <= 0) ? st:((st < deadLine) ? st:-1));
}

bool MonthesNamesParser::initWeekDay(std::string weekDayStr)
{
    // Monday | Tuesday |  ...
    const char* str = weekDayStr.c_str();
    if (!str || str[0] == '\0') return false;
    
    weekDay = scanWeekDay(str);
    if (weekDay < 0 || weekDay > 6) {
        weekDay = 0;
        return false;
    }
    return true;
}
bool MonthesNamesParser::initWeekDayN(std::string weekDayNStr)
{
    // first | second | third | fourth | last.
    const char* str = weekDayNStr.c_str();
    if (!str || str[0] == '\0') return false;
    
    int weekDayN = scanWeekDayN(str);
    if (weekDayN < 0 || weekDayN > 4) {
        weekDayN = 0;
        return false;
    }
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
        if (!initWeekDay(config->getString("weekDay")))
            throw ConfigException("Invalid weekDay parameter, should be one of "
                                  "week days (Mon, Thu, ...).");
        if (!initWeekDayN(config->getString("weekDayN")))
            throw ConfigException("Invalid weekDayN parameter, should be one of: "
                                  "first, second, third, fourth, last.");
    }
    if (!initMonthesNames(config->getString("monthes")))
        throw ConfigException("Invalid monthes parameter, should be "
                              "',' separated list of monthes names.");
}
int getLastMonthDay(tm dt)
{
    for (int i=27; i<32; i++) {
        dt.tm_mday = i;
        (void) mktime(&dt);
        if (dt.tm_mday != i) return i-1;
    }
    return -1;
}
time_t MonthlySchedule::calulateNextTime()
{
    if (startDateTime <= 0 || 
        dayOfMonth == 0 || dayOfMonth > 31) return -1;
    if (dayOfMonth < 0 && 
        (weekDayN < 0 || weekDayN > 4) && (weekDay < 0 || weekDay > 6)) return -1;
    
    time_t ct = time(NULL);
    if (deadLine > 0 && (ct >= deadLine || startDateTime >= deadLine)) return -1;
    
    tm dt; localtime_r(&ct, &dt); // начинаем с текущего времени.
    tm dtst; localtime_r(&startDateTime, &dtst);
    dt.tm_hour = dtst.tm_hour; dt.tm_min = dtst.tm_min; dt.tm_sec = dtst.tm_sec;
    time_t st = mktime(&dt); 

    while (1)
    {
        if (dt.tm_mon < 0 || dt.tm_mon >= 12) return -1;
        
        if (dayOfMonth > 0)
        {
            if (dayOfMonth <= getLastMonthDay(dt))
            {
                dt.tm_mday = dayOfMonth;  // задали день старта
                st = mktime(&dt);        
                if (st > ct && st >= startDateTime) 
                    return ((deadLine <= 0) ? st:((st < deadLine) ? st:-1));
            }
        }
        else
        {
            int lastMonthDay = getLastMonthDay(dt);
            if (weekDayN == 4) // search for last weekDay
            {
                for (dt.tm_mday = lastMonthDay; dt.tm_mday > 0; dt.tm_mday--)
                {
                    st = mktime(&dt); // нормализируем структуру, вычисляем день недели
                    if (dt.tm_wday == ((weekDay == 6) ? 0:weekDay+1))
                    {
                        if (st > ct && st >= startDateTime) 
                            return ((deadLine <= 0) ? st:((st < deadLine) ? st:-1));
                        else break; // try it in next month
                    }
                }
            }
            else // search for first | second | third | fourth weekDay
            {
                int found = 0;
                for (dt.tm_mday = 1; dt.tm_mday <= lastMonthDay; dt.tm_mday++)
                {
                    st = mktime(&dt); // нормализируем структуру, вычисляем день недели
                    if (dt.tm_wday == ((weekDay == 6) ? 0:weekDay+1))
                    {
                        if (found == weekDayN) {
                            if (st > ct && st >= startDateTime) 
                                return ((deadLine <= 0) ? st:((st < deadLine) ? st:-1));
                            else break; // try it in next month
                        }
                        found++;
                    }
                }
            }
        }

        // не нашли в текущем месяце, увеличиваем месяц
        dt.tm_mday = 1;
        int start_mon = dt.tm_mon;                // начинаем с текущего месяца
        while (++dt.tm_mon < start_mon+12) {      // сканируем 12 месяцев после него
            int index = dt.tm_mon-start_mon;
            if (index>=0 && index<12 && monthesNamesSet[index]) break; // выходим если нашли след. месяц
        }
        if (dt.tm_mon >= start_mon+12) return -1; // нечего не нашли => месяцев нет (ошибка инита)

        (void)mktime(&dt); // нормализуем структуру dt, вычисляем нормальный месяц
    }
    
    st = mktime(&dt);
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
    if (startDateTime <= 0) return -1;

    time_t ct = time(NULL);
    time_t st = startDateTime;
    if (deadLine > 0 && ct >= deadLine) return -1;
    if (ct < st) return ((deadLine <= 0) ? st:((st < deadLine) ? st:-1));
    if (intervalTime <= 0) return -1;
    
    st += ((ct-st)/intervalTime + 1)*intervalTime; // no need to ajust daylight
    return ((deadLine <= 0) ? st:((st < deadLine) ? st:-1));
}

}}
