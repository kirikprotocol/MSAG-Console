#ifndef SMSC_INFOSME_ADMIN
#define SMSC_INFOSME_ADMIN

/**
 * 
 * @author Victor V. Makarov
 * @version 1.0
 * @see 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <string>

#include <core/buffers/Array.hpp>

namespace smsc { namespace infosme
{
    using smsc::core::buffers::Array;

    struct InfoSmeAdmin
    {
        virtual void startTaskProcessor() = 0;
        virtual void stopTaskProcessor() = 0;
        virtual bool isTaskProcessorRunning() = 0;

        virtual void startTaskScheduler() = 0;
        virtual void stopTaskScheduler() = 0;
        virtual bool isTaskSchedulerRunning() = 0;

        virtual void flushStatistics() = 0;
        
        virtual bool addTask(std::string taskId) = 0;
        virtual bool removeTask(std::string taskId) = 0; 
        virtual bool changeTask(std::string taskId) = 0;
        
        virtual bool startTask(std::string taskId) = 0;
        virtual bool stopTask(std::string taskId) = 0; 
        virtual Array<std::string> getGeneratingTasks() = 0;
        virtual Array<std::string> getProcessingTasks() = 0;

        virtual bool isTaskEnabled(std::string taskId) = 0;
        virtual bool setTaskEnabled(std::string taskId, bool enabled) = 0;
        
        virtual bool addSchedule(std::string scheduleId) = 0;
        virtual bool removeSchedule(std::string scheduleId) = 0;
        virtual bool changeSchedule(std::string scheduleId) = 0;
        
        InfoSmeAdmin() {};

    protected:

        virtual ~InfoSmeAdmin() {};
    };
        
}}

#endif // SMSC_INFOSME_ADMIN


