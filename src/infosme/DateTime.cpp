
#include "DateTime.h"

namespace smsc { namespace infosme 
{

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
    "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
};
static const char*  constFullEngWeekDayN[5] = {
    "first", "second", "third", "fourth", "last"
};

int scanMonthName(const char* str)
{
    if (!str || str[0] == '\0') return -1;
    for (int i=0; i<12; i++)
        if (strcmp(constFullEngMonthesNames[i], str) == 0 ||
            strcmp(constShortEngMonthesNames[i], str) == 0) return i;
    return -1;    
}
int scanWeekDay(const char* str)
{
    if (!str || str[0] == '\0') return -1;
    for (int i=0; i<7; i++)
        if (strcmp(constFullEngWeekDays[i], str) == 0 ||
            strcmp(constShortEngWeekDays[i], str) == 0) return i;
    return -1;    
}
int scanWeekDayN(const char* str)
{
    if (!str || str[0] == '\0') return -1;
    for (int i=0; i<5; i++)
        if (strcmp(constFullEngWeekDayN[i], str) == 0) return i;
    return -1;    
}

time_t parseDateTime(const char* str)
{
    int year, month, day, hour, minute, second;
    if (!str || str[0] == '\0' ||
        sscanf(str, "%02d.%02d.%4d %02d:%02d:%02d", 
                    &day, &month, &year, &hour, &minute, &second) != 6) return -1;
    
    tm  dt; dt.tm_isdst = -1;
    dt.tm_year = year - 1900; dt.tm_mon = month - 1; dt.tm_mday = day;
    dt.tm_hour = hour; dt.tm_min = minute; dt.tm_sec = second;

    //printf("%02d.%02d.%04d %02d:%02d:%02d = %ld\n", day, month, year, hour, minute, second, time);
    return mktime(&dt);
}
time_t parseDate(const char* str)
{
    int year, month, day;
    if (!str || str[0] == '\0' ||
        sscanf(str, "%02d.%02d.%4d", &day, &month, &year) != 3) return -1;
    
    tm  dt; dt.tm_isdst = -1;
    dt.tm_year = year - 1900; dt.tm_mon = month - 1; dt.tm_mday = day;
    dt.tm_hour = 0; dt.tm_min = 0; dt.tm_sec = 0;
    
    //printf("%02d:%02d:%04d = %ld\n", day, month, year, time);
    return mktime(&dt);
}
int parseTime(const char* str)
{
    int hour, minute, second;
    if (!str || str[0] == '\0' ||
        sscanf(str, "%02d:%02d:%02d", &hour, &minute, &second) != 3) return -1;
    
    //printf("%02d:%02d:%02d = %ld\n", hour, minute, second, time);
    return hour*3600+minute*60+second;
}

int getLastMonthDay(tm dt)
{
    for (int i=27; i<33; i++) {
        dt.tm_mday = i; dt.tm_isdst = -1;
        (void) mktime(&dt);
        if (dt.tm_mday != i) return i-1;
    }
    return -1;
}

bool WeekDaysSet::setWeekDays(const std::string& weekDaysStr)
{
    weekDays = 0;
    // ',' separated list Mon, Tue, ...
    const char* str = weekDaysStr.c_str(); 
    if (!str || str[0] == '\0') return false;
    
    std::string weekDay = "";
    do
    {
        if (*str == ',' || *str == '\0') {
            int day = scanWeekDay(weekDay.c_str());
            if (day < 0 || day > 6) return false;
            weekDays |= 1<<day;
            weekDay = "";
        } 
        else if (!isspace(*str)) weekDay += *str;
    } 
    while (*str++);
    return true;
}
bool WeekDaysSet::isWeekDay(int day)
{
    if (day < 0 || day > 6) return false;
    uint8_t mask = 1<<day;
    return ((weekDays & mask) == mask);
}

bool MonthesNamesParser::initWeekDay(const std::string& weekDayStr)
{
    // Monday | Tuesday |  ...
    weekDay = scanWeekDay(weekDayStr.c_str());
    if (weekDay < 0 || weekDay > 6) {
        weekDay = 0;
        return false;
    }
    return true;
}
bool MonthesNamesParser::initWeekDayN(const std::string& weekDayNStr)
{
    // first | second | third | fourth | last.
    weekDayN = scanWeekDayN(weekDayNStr.c_str());
    if (weekDayN < 0 || weekDayN > 4) {
        weekDayN = 0;
        return false;
    }
    return true;
}
bool MonthesNamesParser::initMonthesNames(const std::string& monthesNames)
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
        else if (!isspace(*str)) monthName += *str;
    } 
    while (*str++);
    return true;
}

}}
