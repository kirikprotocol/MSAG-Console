
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
    try 
    {
        if (schedule) schedule->init(config);
    }
    catch (Exception& exc) {
        if (schedule) delete schedule;
        throw ConfigException("Failed to create schedule '%s'. Reason: %s", id.c_str(), exc.what());
    }
    catch (std::exception& exc) {
        if (schedule) delete schedule;
        throw ConfigException("Failed to create schedule '%s'. Reason: %s", id.c_str(), exc.what());
    }
    catch (...) {
        if (schedule) delete schedule;
        throw ConfigException("Failed to create schedule '%s'. Unknown reason.", id.c_str());
    }
    
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

    const char* tasksStr = config->getString("tasks");
    if (!tasksStr)
        throw ConfigException("Schedule tasks set empty or wasn't specified.");

    const char* tasksCur = tasksStr;
    std::string taskId = "";

    if (*tasksCur != '\0') do
    {
        if (*tasksCur == ',' || *tasksCur == '\0') {
            const char* task_id = taskId.c_str();
            if (!task_id || task_id[0] == '\0')
                throw ConfigException("Task id is invalid.");
            if (!addTask(taskId))
                throw ConfigException("Task '%s' was already assigned to schedule.",task_id);
            taskId = "";
        } 
        else if (!isspace(*tasksCur)) taskId += *tasksCur;
    } 
    while (*tasksCur++);
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
    dt.tm_isdst = -1;
    st = mktime(&dt); 

    return ((deadLine <= 0) ? st:((st < deadLine) ? st:-1));
}

void WeeklySchedule::init(ConfigView* config)
{
    Schedule::init(config, true);

    everyNWeeks = config->getInt("everyNWeeks");
    if (everyNWeeks <= 0)
        throw ConfigException("Invalid everyNWeeks parameter, should be positive.");
    if (!setWeekDays(config->getString("weekDays")))
        throw ConfigException("Invalid weekDays parameter, should be "
                              "',' separated list of week days.");
}
time_t WeeklySchedule::calulateNextTime()
{
    if (startDateTime <= 0) return -1;
    
    time_t ct = time(NULL);
    if (deadLine > 0 && ct >= deadLine) return -1;
    time_t st = startDateTime;
    
    tm dt; localtime_r(&st, &dt);
    
    int intervalInDays = 7*everyNWeeks;
    int intervalInSeconds = intervalInDays*86400;

    if (ct >= st) { // сдвиг до текущего времени
        dt.tm_mday += ((ct-st)/intervalInSeconds)*intervalInDays;
        dt.tm_isdst = -1;
        st = mktime(&dt);
    }
    
    while (1)
    {
        if (deadLine > 0 && st >= deadLine) return -1;
        
        for (int i=0; i<7; i++, dt.tm_mday++)
        {
            dt.tm_isdst = -1;
            st = mktime(&dt);
            //printf("Scanning %s", ctime(&st));
            if (isWeekDay((dt.tm_wday == 0) ? 6:(dt.tm_wday-1)) &&
                st > ct && st >= startDateTime) {
                return ((deadLine <= 0) ? st:((st < deadLine) ? st:-1));
            }
        }
        
        //printf("Switch to next week\n");
        dt.tm_mday += intervalInDays; // Переходим к следующей неделе. 
        dt.tm_isdst = -1;
        st = mktime(&dt); 
    }
    
    return -1;
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
                                  "week days (Mon, Tue, ...).");
        if (!initWeekDayN(config->getString("weekDayN")))
            throw ConfigException("Invalid weekDayN parameter, should be one of: "
                                  "first, second, third, fourth, last.");
    }
    if (!initMonthesNames(config->getString("monthes")))
        throw ConfigException("Invalid monthes parameter, should be "
                              "',' separated list of monthes names.");
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
    dt.tm_isdst = -1;             // меняем время на время запуска.
    time_t st = mktime(&dt); 

    int maxMonthesTries = 12;
    while (maxMonthesTries--)
    {
        dt.tm_mday = 1;
        int start_mon = dt.tm_mon;              // начинаем с текущего месяца
        while (dt.tm_mon < start_mon+12)        // сканируем 12 месяцев после него
            if (monthesNamesSet[dt.tm_mon%12]) break; // выходим если нашли месяц
            else dt.tm_mon++;
        
        if (dt.tm_mon >= start_mon+12) return -1; // нечего не нашли => месяцев нет (ошибка инита)
        dt.tm_isdst = -1;
        st = mktime(&dt); // нормализуем структуру dt, вычисляем нормальный месяц ???
        
        int lastMonthDay = getLastMonthDay(dt);
        if (lastMonthDay < 0 || lastMonthDay > 31) return -1;

        if (dayOfMonth > 0)
        {
            if (dayOfMonth <= lastMonthDay)
            {
                dt.tm_mday = dayOfMonth;  // задали день старта
                dt.tm_isdst = -1;
                st = mktime(&dt);        
                if (st > ct && st >= startDateTime) 
                    return ((deadLine <= 0) ? st:((st < deadLine) ? st:-1));
            }
        }
        else
        {
            //printf("WeekDayN = %d, WeekDay = %d\n", weekDayN, weekDay);
            if (weekDayN == 4) // search for last weekDay
            {
                for (dt.tm_mday = lastMonthDay; dt.tm_mday > 0; dt.tm_mday--)
                {
                    dt.tm_isdst = -1;
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
                    dt.tm_isdst = -1;
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

        dt.tm_mon++; dt.tm_isdst = -1;
        st = mktime(&dt); // нормализируем структуру
    }
    
    return -1;
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
    if (startDateTime <= 0 || intervalTime <= 0) return -1;

    time_t ct = time(NULL);
    time_t st = startDateTime;
    if (deadLine > 0 && ct >= deadLine) return -1;
    if (ct < st) return ((deadLine <= 0) ? st:((st < deadLine) ? st:-1));

    st += ((ct-st)/intervalTime + 1)*intervalTime; // no need to ajust daylight
    return ((deadLine <= 0) ? st:((st < deadLine) ? st:-1));
}

}}
