#ifndef SMSC_INFO_SME_SCHEDULES
#define SMSC_INFO_SME_SCHEDULES

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

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
        
        time_t  startTime;  // only HH:mm:ss
        time_t  startDate;  // only YYYY.MM.dd
        time_t  endDate;    // if -1 not defined, only YYYY.MM.dd
        time_t  endTime;    // if -1 not defined. only HH:mm:ss
        
        Mutex           taskNamesLock;
        Hash<bool>      taskNames;
        
        virtual ~Schedule() {};
        
        virtual time_t calulateNextTime() = 0;
        
        virtual bool addTask(std::string taskName)
        { 
            MutexGuard guard(taskNamesLock);

            const char* task_name = taskName.c_str();
            if (!task_name || task_name[0] == '\0' || taskNames.Exists(task_name)) 
                return false;
            else taskNames.Insert(task_name, true);
            return true;
        };
        virtual bool removeTask(std::string taskName)
        { 
            MutexGuard guard(taskNamesLock);

            const char* task_name = taskName.c_str();
            if (!task_name || task_name[0] == '\0' || !taskNames.Exists(task_name)) 
                return false;
            else taskNames.Delete(task_name);
            return true;
        };

        Hash<bool>& getTasks() {
            MutexGuard guard(taskNamesLock);
            return taskNames;
        }
    
        static Schedule* create(ConfigView* config, std::string id);
        virtual void init(ConfigView* config) = 0;
        void init(ConfigView* config, bool full);

    protected:
        
        Schedule(std::string id, ScheduleType type, 
                 time_t startTime=-1, time_t startDate=-1, time_t endTime=-1, time_t endDate=-1) 
            : id(id), type(type), 
              startTime(startTime), startDate(startDate), endTime(endTime), endDate(endDate) {};
    };

    struct OnceSchedule : public Schedule
    {
        OnceSchedule(std::string id)
            : Schedule(id, ONCE) {};

        OnceSchedule(std::string id, time_t startTime, time_t startDate)
            : Schedule(id, ONCE, startTime, startDate, -1, -1) {};
        
        virtual void init(ConfigView* config);
        virtual time_t calulateNextTime();
    };
    
    struct DailySchedule : public Schedule
    {
        int     everyNDays;

        DailySchedule(std::string id)
            : Schedule(id, DAILY), everyNDays(0) {};
        
        DailySchedule(std::string id, time_t startTime, time_t startDate, int everyNDays,
                      time_t endTime=-1, time_t endDate=-1)
            : Schedule(id, DAILY, startTime, startDate, endTime, endDate), 
              everyNDays(everyNDays) {};
        
        virtual void init(ConfigView* config);
        virtual time_t calulateNextTime();
    };
    
    struct WeeklySchedule : public Schedule
    {
        int         everyNWeeks;
        std::string weekDays;   // ',' separated list Mon, Thu, ...
        
        int         weekDaysSet[7];

        WeeklySchedule(std::string id)
            : Schedule(id, WEEKLY), everyNWeeks(0), weekDays("") {};
        
        WeeklySchedule(std::string id, time_t startTime, time_t startDate,
                       int everyNWeeks, std::string weekDays,
                       time_t endTime=-1, time_t endDate=-1)
            : Schedule(id, WEEKLY, startTime, startDate, endTime, endDate),
              everyNWeeks(everyNWeeks), weekDays(weekDays){};
        
        virtual void init(ConfigView* config);
        virtual time_t calulateNextTime();
    };
    
    struct MonthlySchedule : public Schedule
    {
        int         dayOfMonth;  // if -1 using weeks
        std::string weekN;       // ',' separated list first, second, third, fourth, last.
        std::string weekDays;    // ',' separated list Mon, Thu, ...  
        std::string monthes;     // ',' separated list Jan, Feb, ...

        MonthlySchedule(std::string id)
            : Schedule(id, MONTHLY), dayOfMonth(1), weekN(""), weekDays(""), monthes("") {};

        MonthlySchedule(std::string id, time_t startTime, time_t startDate, int dayOfMonth, 
                        std::string weekN, std::string weekDays, std::string monthes,
                        time_t endTime=-1, time_t endDate=-1)
            : Schedule(id, MONTHLY, startTime, startDate, endTime, endDate),
              dayOfMonth(dayOfMonth), weekN(weekN), weekDays(weekDays), monthes(monthes) {};
        
        virtual void init(ConfigView* config);
        virtual time_t calulateNextTime();
    };

    struct IntervalSchedule : public Schedule
    {
        time_t  intervalTime; // only HH:mm:ss

        IntervalSchedule(std::string id)
            : Schedule(id, INTERVAL), intervalTime(0) {};

        IntervalSchedule(std::string id, time_t startTime, time_t startDate, 
                         time_t intervalTime, time_t endTime=-1, time_t endDate=-1)
            : Schedule(id, INTERVAL, startTime, startDate, endTime, endDate),
              intervalTime(intervalTime) {};
        
        virtual void init(ConfigView* config);
        virtual time_t calulateNextTime();
    };
    
}}

#endif //SMSC_INFO_SME_SCHEDULES
