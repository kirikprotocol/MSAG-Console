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

#include "TaskTypes.hpp"

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
        
        virtual void addTask(uint32_t taskId) = 0;
        virtual void removeTask(uint32_t taskId) = 0; 
        virtual void changeTask(uint32_t taskId) = 0;

        virtual TaskInfo getTaskInfo(uint32_t taskId)=0;
        
        virtual bool startTask(uint32_t taskId) = 0;
        virtual bool stopTask(uint32_t taskId) = 0; 
        virtual Array<std::string> getGeneratingTasks() = 0;
        virtual Array<std::string> getProcessingTasks() = 0;

        virtual bool isTaskEnabled(uint32_t taskId) = 0;
        virtual bool setTaskEnabled(uint32_t taskId, bool enabled) = 0;
        
        virtual void addSchedule(std::string scheduleId) = 0;
        virtual void removeSchedule(std::string scheduleId) = 0;
        virtual void changeSchedule(std::string scheduleId) = 0;
        virtual void addDeliveryMessages(uint32_t taskId,
                                         uint8_t msgState,
                                         const std::string& address,
                                         time_t messageDate,
                                         const std::string& msg) = 0;

        virtual void changeDeliveryMessageInfoByRecordId(uint32_t taskId,
                                                         uint8_t messageState,
                                                         time_t unixTime,
                                                         const std::string& recordId) = 0;

        virtual void changeDeliveryMessageInfoByCompositCriterion(uint32_t taskId,
                                                                  uint8_t messageState,
                                                                  time_t unixTime,
                                                                  const InfoSme_T_SearchCriterion& searchCrit) = 0;
        virtual void deleteDeliveryMessageByRecordId(uint32_t taskId,
                                                     const std::string& recordId) = 0;

        virtual void deleteDeliveryMessagesByCompositCriterion(uint32_t taskId,
                                                               const InfoSme_T_SearchCriterion& searchCrit) = 0;

        /*virtual void insertRecordIntoTasksStat(uint32_t taskId,
                                               uint32_t period,
                                               uint32_t generated,
                                               uint32_t delivered,
                                               uint32_t retried,
                                               uint32_t failed) = 0;*/

        virtual Array<std::string> getTaskMessages(uint32_t taskId,
                                                   const InfoSme_T_SearchCriterion& searchCrit) = 0;

        //virtual Array<std::string> getTasksStatistic(const InfoSme_Tasks_Stat_SearchCriterion& searchCrit) = 0;

        virtual void endDeliveryMessagesGeneration(uint32_t taskId) = 0;

        virtual void changeDeliveryTextMessageByCompositCriterion(uint32_t taskId,
                                                                  const std::string& newTextMsg,
                                                                  const InfoSme_T_SearchCriterion& searchCrit) = 0;
        InfoSmeAdmin() {};

    protected:

        virtual ~InfoSmeAdmin() {};
    };
        
}}

#endif // SMSC_INFOSME_ADMIN


