#ifndef SMSC_INFO_SME_SCHEDULES
#define SMSC_INFO_SME_SCHEDULES

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <set>

#include <core/synchronization/Mutex.hpp>

#include <util/config/Config.h>
#include <util/config/Manager.h>
#include <util/config/ConfigView.h>
#include <util/config/ConfigException.h>

#include "Task.h"

namespace smsc { namespace infosme 
{
    using smsc::core::synchronization::Mutex;
    
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;
    
    const uint8_t BYTE_ONCE_TYPE = (uint8_t)0;
    const uint8_t BYTE_DAILY_TYPE = (uint8_t)1;
    const uint8_t BYTE_WEEKLY_TYPE = (uint8_t)2;
    const uint8_t BYTE_MONTHLY_TYPE = (uint8_t)3;
    const uint8_t BYTE_INTERVAL_TYPE = (uint8_t)4;
    
    typedef enum {
        ONCE = BYTE_ONCE_TYPE, 
        DAILY = BYTE_DAILY_TYPE, 
        WEEKLY = BYTE_WEEKLY_TYPE, 
        MONTHLY = BYTE_MONTHLY_TYPE,
        INTERVAL = BYTE_INTERVAL_TYPE
    } ScheduleType;

    static const char* SCHEDULE_TYPE_ONCE = "once";
    static const char* SCHEDULE_TYPE_DAILY = "daily";
    static const char* SCHEDULE_TYPE_WEEKLY = "weekly";
    static const char* SCHEDULE_TYPE_MONTHLY = "monthly";
    static const char* SCHEDULE_TYPE_INTERVAL = "interval";

    struct Schedule
    {
        std::string     id;
        ScheduleType    type;
        
        
        time_t  startDateTime;  // full YYYY.MM.dd HH:mm:ss
        time_t  deadLine;       // full YYYY.MM.dd HH:mm:ss
        
        Mutex           taskIdsLock;
        typedef std::set<uint32_t> IntSet;
        IntSet taskIds;
        
        virtual ~Schedule() {};
        
        virtual time_t calulateNextTime() = 0;
        
        virtual bool addTask(uint32_t taskId)
        { 
            MutexGuard guard(taskIdsLock);
            taskIds.insert(taskId);
            return true;
        };
        virtual bool removeTask(uint32_t taskId)
        { 
            MutexGuard guard(taskIdsLock);

            IntSet::iterator it=taskIds.find(taskId);
            if (it==taskIds.end()) 
            {
              return false;
            }
            else
            {
              taskIds.erase(it);
            }
            return true;
        };

        IntSet getTasks()
        {
          MutexGuard guard(taskIdsLock);
          return taskIds;
        }
    
        static Schedule* create(ConfigView* config, std::string id);
        virtual void init(ConfigView* config) = 0;
        void baseinit(ConfigView* config, bool full);

    protected:
        
        Schedule(std::string id, ScheduleType type, time_t startDateTime=-1, time_t endDateTime=-1) 
            : id(id), type(type), startDateTime(startDateTime), deadLine(endDateTime) {};
    };

    struct OnceSchedule : public Schedule
    {
        OnceSchedule(std::string id)
            : Schedule(id, ONCE) {};

        OnceSchedule(std::string id, time_t startDateTime)
            : Schedule(id, ONCE, startDateTime, -1) {};
        
        virtual void init(ConfigView* config);
        virtual time_t calulateNextTime();
    };
    
    struct DailySchedule : public Schedule
    {
        int     everyNDays;

        DailySchedule(std::string id)
            : Schedule(id, DAILY), everyNDays(0) {};
        
        DailySchedule(std::string id, time_t startDateTime, int everyNDays,
                      time_t endDateTime=-1)
            : Schedule(id, DAILY, startDateTime, endDateTime), 
              everyNDays(everyNDays) {};
        
        virtual void init(ConfigView* config);
        virtual time_t calulateNextTime();
    };
    
    struct WeeklySchedule : public Schedule, public WeekDaysSet
    {
        int         everyNWeeks;
        
        WeeklySchedule(std::string id)
            : Schedule(id, WEEKLY), WeekDaysSet(), everyNWeeks(0) {};
        
        WeeklySchedule(std::string id, time_t startDateTime,
                       int everyNWeeks, std::string weekDays,
                       time_t endDateTime=-1)
            : Schedule(id, WEEKLY, startDateTime, endDateTime),
              WeekDaysSet(weekDays), everyNWeeks(everyNWeeks) {};
        
        virtual void init(ConfigView* config);
        virtual time_t calulateNextTime();
    };
    
    struct MonthlySchedule : public Schedule, public MonthesNamesParser
    {
        int         dayOfMonth;  // if -1 using weeks

        MonthlySchedule(std::string id)
            : Schedule(id, MONTHLY), MonthesNamesParser(), dayOfMonth(1) {};

        MonthlySchedule(std::string id, time_t startDateTime, int dayOfMonth, 
                        std::string weekDayN, std::string weekDay, std::string monthesNames,
                        time_t endDateTime=-1)
            : Schedule(id, MONTHLY, startDateTime, endDateTime),
              MonthesNamesParser(weekDay, weekDayN, monthesNames), dayOfMonth(dayOfMonth) {};
        
        virtual void init(ConfigView* config);
        virtual time_t calulateNextTime();
    };

    struct IntervalSchedule : public Schedule
    {
        int     intervalTime; // only HH:mm:ss

        IntervalSchedule(std::string id)
            : Schedule(id, INTERVAL), intervalTime(0) {};

        IntervalSchedule(std::string id, time_t startDateTime, 
                         int intervalTime, time_t endDateTime=-1)
            : Schedule(id, INTERVAL, startDateTime, endDateTime),
              intervalTime(intervalTime) {};
        
        virtual void init(ConfigView* config);
        virtual time_t calulateNextTime();
    };
    
}}

#endif //SMSC_INFO_SME_SCHEDULES
