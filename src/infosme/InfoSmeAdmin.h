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

#include "InfoSme_T_SearchCriterion.hpp"
#include "InfoSme_Tasks_Stat_SearchCriterion.hpp"

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
        
        virtual void addTask(std::string taskId) = 0;
        virtual void removeTask(std::string taskId) = 0; 
        virtual void changeTask(std::string taskId) = 0;
        
        virtual bool startTask(std::string taskId) = 0;
        virtual bool stopTask(std::string taskId) = 0; 
        virtual Array<std::string> getGeneratingTasks() = 0;
        virtual Array<std::string> getProcessingTasks() = 0;

        virtual bool isTaskEnabled(std::string taskId) = 0;
        virtual bool setTaskEnabled(std::string taskId, bool enabled) = 0;
        
        virtual void addSchedule(std::string scheduleId) = 0;
        virtual void removeSchedule(std::string scheduleId) = 0;
        virtual void changeSchedule(std::string scheduleId) = 0;
        virtual void addDeliveryMessages(const std::string& taskId,
                                         uint8_t msgState,
                                         const std::string& address,
                                         time_t messageDate,
                                         const std::string& msg) = 0;

        virtual void changeDeliveryMessageInfoByRecordId(const std::string& taskId,
                                                         uint8_t messageState,
                                                         time_t unixTime,
                                                         const std::string& recordId) = 0;

        virtual void changeDeliveryMessageInfoByCompositCriterion(const std::string& taskId,
                                                                  uint8_t messageState,
                                                                  time_t unixTime,
                                                                  const InfoSme_T_SearchCriterion& searchCrit) = 0;
        virtual void deleteDeliveryMessageByRecordId(const std::string& taskId,
                                                     const std::string& recordId) = 0;

        virtual void deleteDeliveryMessagesByCompositCriterion(const std::string& taskId,
                                                               const InfoSme_T_SearchCriterion& searchCrit) = 0;

        virtual void insertRecordIntoTasksStat(const std::string& taskId,
                                               uint32_t period,
                                               uint32_t generated,
                                               uint32_t delivered,
                                               uint32_t retried,
                                               uint32_t failed) = 0;

        virtual Array<std::string> getTaskMessages(const std::string& taskId,
                                                   const InfoSme_T_SearchCriterion& searchCrit) = 0;

        virtual Array<std::string> getTasksStatistic(const InfoSme_Tasks_Stat_SearchCriterion& searchCrit) = 0;

        virtual void endDeliveryMessagesGeneration(const std::string& taskId) = 0;
        InfoSmeAdmin() {};

    protected:

        virtual ~InfoSmeAdmin() {};
    };
        
}}

#endif // SMSC_INFOSME_ADMIN


