#ifndef SMSC_INFO_SME_SCHEDULES
#define SMSC_INFO_SME_SCHEDULES

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <core/synchronization/Mutex.hpp>

#include "Task.h"

namespace smsc { namespace infosme 
{
    using smsc::core::synchronization::Mutex;
    
    const uint8_t BYTE_ONCE_TYPE       = (uint8_t)0;
    const uint8_t BYTE_DAILY_TYPE      = (uint8_t)1;
    const uint8_t BYTE_WEEKLY_TYPE     = (uint8_t)2;
    const uint8_t BYTE_MONTHLY_TYPE    = (uint8_t)3;
    
    typedef enum {
        ONCE    = BYTE_ONCE_TYPE, 
        DAILY   = BYTE_DAILY_TYPE, 
        WEEKLY  = BYTE_WEEKLY_TYPE, 
        MONTHLY = BYTE_MONTHLY_TYPE
    } ScheduleType;

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
        int             id;
        ScheduleType    type;
        Advanced        advanced;

        Task*           task;
        
        virtual ~Schedule() {};
        
        virtual time_t calulateNextTime() = 0;

        void assignTask(Task* task) { this->task = task; };
        void assignAdvanced(const Advanced& advanced) { this->advanced = advanced; };
    
    protected:
        Schedule(ScheduleType type) 
            : id(Schedule::getNextId()), type(type), task(0) {};
    private:
        static int getNextId()
        {
            static Mutex getNextIdMutex;
            static int currentId = 0;
            
            MutexGuard guard(getNextIdMutex);
            return currentId++;
        };
    };

    struct OnceSchedule : public Schedule
    {
        time_t  startTime;  // only HH:mm:ss
        time_t  startDate;  // YYYY.MM.dd
        
        // Has no startTime & endDate in advanced
        
        OnceSchedule(time_t startTime, time_t startDate)
            : Schedule(ONCE), 
              startTime(startTime), startDate(startDate) {};

        virtual time_t calulateNextTime();
    };
    
    struct DailySchedule : public Schedule
    {
        time_t  startTime;  // only HH:mm:ss
        int     everyNDays;

        DailySchedule(time_t startTime, int everyNDays)
            : Schedule(DAILY), 
              startTime(startTime), everyNDays(everyNDays) {};

        virtual time_t calulateNextTime();
    };
    
    struct WeeklySchedule : public Schedule
    {
        time_t      startTime;  // only HH:mm:ss
        int         everyNWeeks;
        std::string weekDays;   // ',' separated list Mon, Thu, ...

        WeeklySchedule(time_t startTime, int everyNWeeks, std::string weekDays)
            : Schedule(WEEKLY), 
              startTime(startTime), everyNWeeks(everyNWeeks), weekDays(weekDays) {};
        
        virtual time_t calulateNextTime();
    };
    
    struct MonthlySchedule : public Schedule
    {
        time_t      startTime;   // only HH:mm:ss
        int         dayOfMonth; // if -1 using weeks
        std::string weekN;       // ',' separated list first, second, third, fourth, last.
        std::string weekDays;    // ',' separated list Mon, Thu, ...  
        std::string monthDays;   // ',' separated list Jan, Feb, ...

        MonthlySchedule(time_t startTime, int dayOfMonth, 
                        std::string weekN, std::string weekDays, std::string monthDays)
            : Schedule(MONTHLY), 
              startTime(startTime), dayOfMonth(dayOfMonth),
              weekN(weekN), weekDays(weekDays), monthDays(monthDays) {};

        virtual time_t calulateNextTime();
    };

}}

#endif //SMSC_INFO_SME_SCHEDULES
