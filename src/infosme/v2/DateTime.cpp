#include "DateTime.h"
#include <sstream>
#include <iomanip>
#include <cerrno>

namespace smsc {
namespace infosme2 {

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
        if ( std::strcmp(constFullEngMonthesNames[i], str) == 0 ||
             std::strcmp(constShortEngMonthesNames[i], str) == 0) return i;
    return -1;    
}
int scanWeekDay(const char* str)
{
    if (!str || str[0] == '\0') return -1;
    for (int i=0; i<7; i++)
        if (std::strcmp(constFullEngWeekDays[i], str) == 0 ||
            std::strcmp(constShortEngWeekDays[i], str) == 0) return i;
    return -1;    
}
int scanWeekDayN(const char* str)
{
    if (!str || str[0] == '\0') return -1;
    for (int i=0; i<5; i++)
        if (std::strcmp(constFullEngWeekDayN[i], str) == 0) return i;
    return -1;    
}

time_t parseDateTime(const char* str)
{
    int year, month, day, hour, minute, second;
    if (!str || str[0] == '\0' ||
        std::sscanf(str, "%02d.%02d.%4d %02d:%02d:%02d", 
                    &day, &month, &year, &hour, &minute, &second) != 6) return -1;
    
    tm  dt; dt.tm_isdst = -1;
    dt.tm_year = year - 1900; dt.tm_mon = month - 1; dt.tm_mday = day;
    dt.tm_hour = hour; dt.tm_min = minute; dt.tm_sec = second;
    return mktime(&dt);
}
time_t parseDate(const char* str)
{
    int year, month, day;
    if (!str || str[0] == '\0' ||
        std::sscanf(str, "%02d.%02d.%4d", &day, &month, &year) != 3) return -1;
    
    tm  dt; dt.tm_isdst = -1;
    dt.tm_year = year - 1900; dt.tm_mon = month - 1; dt.tm_mday = day;
    dt.tm_hour = 0; dt.tm_min = 0; dt.tm_sec = 0;
    return mktime(&dt);
}
int parseTime(const char* str)
{
    int hour, minute, second;
    if (!str || str[0] == '\0' ||
        std::sscanf(str, "%02d:%02d:%02d", &hour, &minute, &second) != 3) return -1;
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
    std::memset(&monthesNamesSet, 0, sizeof(monthesNamesSet));

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

/*
** Convert unix epoche time to string presentation in DDMMYYYYHH24MISS format
*/
std::string unixTimeToStringFormat(time_t aTime)
{
  struct tm* tmPtr;
  struct tm  bufForDate;

  tmPtr = localtime_r(&aTime, &bufForDate);

  std::ostringstream dateStrBuf;
  dateStrBuf << std::setw(2) << std::setfill('0') 
       << uint16_t(tmPtr->tm_mday)
             << std::setw(2)
             << uint16_t(tmPtr->tm_mon + 1)
       << std::setw(4)
             << uint32_t(tmPtr->tm_year + 1900)
             << std::setw(2)
             << uint16_t(tmPtr->tm_hour)
             << std::setw(2)
             << uint16_t(tmPtr->tm_min)
             << std::setw(2)
             << uint16_t(tmPtr->tm_sec);

  return dateStrBuf.str();
}

/*
** Convert string in DDMMYYYYHH24MISS format to unix epoche time.
*/
bool convertFullDateFormatToUnixTime(// expected date format is DDMMYYYYHH24MISS
                                     const std::string& date,
                                     time_t* unixTime)
{
  struct tm timeval;
  errno = 0;
  timeval.tm_mday = (int)std::strtol(date.substr(0, 2).c_str(), (char **)NULL, 10);
  if ( (!timeval.tm_mday && errno) ||
       (timeval.tm_mday > 31 || timeval.tm_mday  < 1 ) )
       return false;

  timeval.tm_mon=(int)std::strtol(date.substr(2, 2).c_str(), (char **)NULL, 10);
  if ( (!timeval.tm_mon && errno) ||
       (timeval.tm_mon>12 || timeval.tm_mon<1 ) )
    return false;
  timeval.tm_mon = timeval.tm_mon - 1;

  timeval.tm_year = (int)std::strtol(date.substr(4, 4).c_str(), (char**)NULL, 10);
  if ( (!timeval.tm_year && errno) ||
       timeval.tm_year < 1900 )
    return false;
  timeval.tm_year = timeval.tm_year - 1900;

  timeval.tm_hour = (int)std::strtol(date.substr(8, 2).c_str(), (char**)NULL, 10);
  if ( (!timeval.tm_hour && errno) ||
       ( timeval.tm_hour < 0 || timeval.tm_hour > 23 ) )
    return false;

  timeval.tm_min = (int)std::strtol(date.substr(10, 2).c_str(), (char**)NULL, 10);
  if ( (!timeval.tm_min && errno) ||
       ( timeval.tm_min < 0 || timeval.tm_min > 59 ) )
    return false;

  timeval.tm_sec = (int)std::strtol(date.substr(12, 2).c_str(), (char**)NULL, 10);
  if ( (!timeval.tm_sec && errno) ||
       ( timeval.tm_sec < 0 || timeval.tm_sec > 61 ) )
    return false;

  timeval.tm_isdst = -1;
  *unixTime = mktime(&timeval);
  if ( *unixTime == -1 && errno )
    return false;

  return true;
}

}}
