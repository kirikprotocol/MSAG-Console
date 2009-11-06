#ifndef SMSC_INFOSME2_COMPONENT
#define SMSC_INFOSME2_COMPONENT

#include "logger/Logger.h"
// #include "util/Exception.hpp"

#include "admin/service/Component.h"
#include "admin/service/Method.h"
// #include "admin/service/Type.h"

#include "InfoSmeAdmin.h"
// #include "sme/SmppBase.hpp"

namespace smsc {
namespace infosme2 {

class InfoSmeComponent : public smsc::admin::service::Component
{
private:
    enum {
        startTaskProcessorMethod, stopTaskProcessorMethod, isTaskProcessorRunningMethod,
        startTaskSchedulerMethod, stopTaskSchedulerMethod, isTaskSchedulerRunningMethod,
        flushStatisticsMethod, 
        addTaskMethod, removeTaskMethod, changeTaskMethod, 
        startTasksMethod, stopTasksMethod, 
        getGeneratingTasksMethod, getProcessingTasksMethod,
        isTaskEnabledMethod, setTaskEnabledMethod, 
        addScheduleMethod, removeScheduleMethod, changeScheduleMethod,
        addDeliveryMessagesMethod, changeDeliveryMessageInfoMethod,
        deleteDeliveryMessagesMethod, addStatisticRecordMethod,
        selectTaskMessagesMethod, selectTasksStatisticMethod, endDeliveryMessagesGenerationMethod,
        changeDeliveryTextMessageMethod,applyRetryPoliciesMethod,
        sendSmsMethod, reloadSmscAndRegionsMethod
    };
      
public:
    InfoSmeComponent(InfoSmeAdmin& admin);
    virtual ~InfoSmeComponent();
      
    virtual const char* const getName() const {
        return "InfoSme";
    }
    virtual const smsc::admin::service::Methods& getMethods() const {
        return methods_;
    }
    virtual smsc::admin::service::Variant 
        call( const smsc::admin::service::Method& method,
              const smsc::admin::service::Arguments& args ) 
            throw (smsc::admin::service::AdminException);

protected:
    /*
    void addTask(const Arguments& args);
    void removeTask(const Arguments& args);
    void changeTask(const Arguments& args);
      
    void startTasks(const Arguments& args);
    void stopTasks(const Arguments& args); 
    Variant getGeneratingTasks(const Arguments& args);
    Variant getProcessingTasks(const Arguments& args);
      
    void setTaskEnabled(const Arguments& args);
    bool isTaskEnabled(const Arguments& args);
      
    void addSchedule(const Arguments& args);
    void removeSchedule(const Arguments& args);
    void changeSchedule(const Arguments& args);
      
    Variant sendSms(const Arguments& args);
      
    void applyRetryPolicies(const Arguments& args);

    void addDeliveryMessages(const Arguments& args);
    void changeDeliveryMessageInfo(const Arguments& args);
    void deleteDeliveryMessages(const Arguments& args);
    //void addStatisticRecord(const Arguments& args);
    Variant selectTaskMessages(const Arguments& args);
    Variant selectTasksStatistic(const Arguments& args);
    void endDeliveryMessagesGeneration(const Arguments& args);
    void changeDeliveryTextMessage(const Arguments& args);
    void reloadSmscAndRegions(const Arguments& args);
     */

private:
    /*
    void error(const char* method, const char* param);
    void deleteEscapeSymbols(std::string* message);
      
    void splitMessageToFields(const std::string& messageDescription, long* messageState,
                              std::string* address, std::string* messageDate,
                              std::string* messageText, std::string* userData );
      
    bool getParameterIfExistsAndNotNull(const Arguments& args, const char* argumentId,
                                        std::string& argumentValue);
     */

private:
    smsc::logger::Logger*         log_;
    InfoSmeAdmin&                 admin_;
    smsc::admin::service::Methods methods_;
};

}
}

#endif // ifndef SMSC_INFOSME_COMPONENT
