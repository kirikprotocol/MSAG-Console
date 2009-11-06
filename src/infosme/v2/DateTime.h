#ifndef SMSC_INFOSME2_DATETIME
#define SMSC_INFOSME2_DATETIME

#include <sys/types.h>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "util/int.h"

#include <string>

namespace smsc {
namespace infosme2 {

    extern time_t parseDateTime(const char* str);
    extern time_t parseDate(const char* str);
    extern int    parseTime(const char* str);
    
    extern int getLastMonthDay(tm dt);

    extern int scanMonthName(const char* str);
    extern int scanWeekDay(const char* str);
    extern int scanWeekDayN(const char* str);
    /*
    ** Convert string in DDMMYYYYHH24MISS format to unix epoche time.
    */
    extern bool convertFullDateFormatToUnixTime(// expected date format is DDMMYYYYHH24MISS
                                                const std::string& date,
                                                time_t* unixTime);
    /*
    ** Convert unix epoche time to string presentation in DDMMYYYYHH24MISS format
    */
    extern std::string unixTimeToStringFormat(time_t aTime);

    struct WeekDaysSet
    {
        uint8_t weekDays;
        
        WeekDaysSet(uint8_t _weekDays=0) : weekDays(_weekDays) {};
        WeekDaysSet(const WeekDaysSet& set) : weekDays(set.weekDays) {};
        WeekDaysSet(const std::string& weekDaysStr) { 
            setWeekDays(weekDaysStr);
        };

        WeekDaysSet& operator=(const WeekDaysSet& set) { 
            weekDays = set.weekDays;
            return (*this);
        };
        
        bool setWeekDays(const std::string& weekDays); // ',' separated list Mon, Tue, ...
        bool isWeekDay(int day); // Mon=0, Tue=1...
    };

    class MonthesNamesParser
    {
    protected:
        
        int  weekDayN, weekDay;
        bool monthesNamesSet[12];
        
    public:
        
        MonthesNamesParser() : weekDayN(0), weekDay(0)
        {
            std::memset(&monthesNamesSet, 0, sizeof(monthesNamesSet));
        };
        MonthesNamesParser(const std::string& _weekDay,
                           const std::string& _weekDayN,
                           const std::string& _monthesNames) : weekDayN(0), weekDay(0)
        {
            initWeekDay(_weekDay);
            initWeekDayN(_weekDayN);
            initMonthesNames(_monthesNames);
        };
        virtual ~MonthesNamesParser() {};

        bool initWeekDay(const std::string& weekDay);           // Mon | Tue | ...
        bool initWeekDayN(const std::string& weekDayN);         // second | third | fourth | last.
        bool initMonthesNames(const std::string& monthesNames); // ',' separated list Jan, Feb, ...
    };
    
}}

#endif //SMSC_INFO_SME_DATETIME
