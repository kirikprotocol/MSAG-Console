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
    
    const uint8_t BYTE_ONCE_TYPE        = (uint8_t)0;
    const uint8_t BYTE_DAILY_TYPE       = (uint8_t)1;
    const uint8_t BYTE_WEEKLY_TYPE      = (uint8_t)2;
    const uint8_t BYTE_MONTHLY_TYPE     = (uint8_t)3;
    
    typedef enum {
        ONCE    = BYTE_ONCE_TYPE, 
        DAILY   = BYTE_DAILY_TYPE, 
        WEEKLY  = BYTE_WEEKLY_TYPE, 
        MONTHLY = BYTE_MONTHLY_TYPE
    } ScheduleType;

    static const char* SCHEDULE_TYPE_ONCE      = "once";
    static const char* SCHEDULE_TYPE_DAILY     = "daily";
    static const char* SCHEDULE_TYPE_WEEKLY    = "weekly";
    static const char* SCHEDULE_TYPE_MONTHLY   = "monthly";

    struct Advanced
    {
        time_t  startDate;  // if -1 not defined, only YYYY.MM.dd
        time_t  endDate;    // if -1 not defined, only YYYY.MM.dd
        bool    repeat;     // default false, following used if repeat == true
        int     everyNSec;  // if -1 not defined. In seconds (minutes, hours), 
        time_t  endTime;    // if -1 not defined. only HH:mm:ss

        Advanced(time_t startDate = -1, time_t endDate = -1, bool repeat=false, 
                 int everyNSec = -1, time_t endTime = -1)
            : startDate(startDate), endDate(endDate), repeat(repeat),
              everyNSec(everyNSec), endTime(endTime) {};
    };

    struct Schedule
    {
        std::string     id;
        ScheduleType    type;
        Advanced        advanced;

        Mutex           taskNamesLock;
        Hash<bool>      taskNames;
        
        virtual ~Schedule() {};
        
        virtual time_t calulateNextTime() = 0;
        
        void assignAdvanced(const Advanced& advanced) { this->advanced = advanced; };
        
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

    protected:
        
        Schedule(std::string id, ScheduleType type) 
            : id(id), type(type) {};
    };

    struct OnceSchedule : public Schedule
    {
        time_t  startTime;  // only HH:mm:ss
        time_t  startDate;  // YYYY.MM.dd
        
        // Has no startTime & endDate in advanced
        
        OnceSchedule(std::string id)
            : Schedule(id, ONCE), 
              startTime(-1), startDate(-1) {};

        OnceSchedule(std::string id, time_t startTime, time_t startDate)
            : Schedule(id, ONCE), 
              startTime(startTime), startDate(startDate) {};
        
        virtual void init(ConfigView* config);
        virtual time_t calulateNextTime();
    };
    
    struct DailySchedule : public Schedule
    {
        time_t  startTime;  // only HH:mm:ss
        int     everyNDays;

        DailySchedule(std::string id)
            : Schedule(id, DAILY), 
              startTime(-1), everyNDays(0) {};
        
        DailySchedule(std::string id, time_t startTime, int everyNDays)
            : Schedule(id, DAILY), 
              startTime(startTime), everyNDays(everyNDays) {};
        
        virtual void init(ConfigView* config);
        virtual time_t calulateNextTime();
    };
    
    struct WeeklySchedule : public Schedule
    {
        time_t      startTime;  // only HH:mm:ss
        int         everyNWeeks;
        std::string weekDays;   // ',' separated list Mon, Thu, ...

        WeeklySchedule(std::string id)
            : Schedule(id, WEEKLY), 
              startTime(-1), everyNWeeks(0), weekDays("") {};
        
        WeeklySchedule(std::string id, time_t startTime, int everyNWeeks, std::string weekDays)
            : Schedule(id, WEEKLY), 
              startTime(startTime), everyNWeeks(everyNWeeks), weekDays(weekDays) {};
        
        virtual void init(ConfigView* config);
        virtual time_t calulateNextTime();
    };
    
    struct MonthlySchedule : public Schedule
    {
        time_t      startTime;   // only HH:mm:ss
        int         dayOfMonth;  // if -1 using weeks
        std::string weekN;       // ',' separated list first, second, third, fourth, last.
        std::string weekDays;    // ',' separated list Mon, Thu, ...  
        std::string monthDays;   // ',' separated list Jan, Feb, ...

        MonthlySchedule(std::string id)
            : Schedule(id, MONTHLY), 
              startTime(-1), dayOfMonth(1),
              weekN(""), weekDays(""), monthDays("") {};

        MonthlySchedule(std::string id, time_t startTime, int dayOfMonth, 
                        std::string weekN, std::string weekDays, std::string monthDays)
            : Schedule(id, MONTHLY), 
              startTime(startTime), dayOfMonth(dayOfMonth),
              weekN(weekN), weekDays(weekDays), monthDays(monthDays) {};
        
        virtual void init(ConfigView* config);
        virtual time_t calulateNextTime();
    };

}}

#endif //SMSC_INFO_SME_SCHEDULES
