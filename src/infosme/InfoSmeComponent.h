#ifndef SMSC_INFOSME_COMPONENT
#define SMSC_INFOSME_COMPONENT

#include <logger/Logger.h>
#include <util/Exception.hpp>

#include <admin/service/Component.h>
#include <admin/service/Method.h>
#include <admin/service/Type.h>

#include "InfoSmeAdmin.h"

namespace smsc { namespace infosme
{
    using namespace smsc::admin::service;
    
    using smsc::util::Logger;
    using smsc::util::Exception;
    
    class InfoSmeComponent : public Component
    {
    private:
    
        log4cpp::Category  &logger;

        InfoSmeAdmin       &admin;
        Methods            methods;
        
        enum {
            startTaskProcessorMethod, stopTaskProcessorMethod, isTaskProcessorRunningMethod,
            startTaskSchedulerMethod, stopTaskSchedulerMethod, isTaskSchedulerRunningMethod,
            flushStatisticsMethod, 
            addTasksMethod, removeTasksMethod, changeTasksMethod, 
            startTasksMethod, stopTasksMethod, 
            getGeneratingTasksMethod, getProcessingTasksMethod,
            isTaskEnabledMethod, setTaskEnabledMethod, 
            addSchedulesMethod, removeSchedulesMethod, changeSchedulesMethod
        };
        
        void error(const char* method, const char* param);
    
    protected:

        void addTasks(const Arguments& args);
        void removeTasks(const Arguments& args);
        void changeTasks(const Arguments& args);
        
        void startTasks(const Arguments& args);
        void stopTasks(const Arguments& args); 
        Variant getGeneratingTasks(const Arguments& args);
        Variant getProcessingTasks(const Arguments& args);

        void setTaskEnabled(const Arguments& args);
        bool isTaskEnabled(const Arguments& args);
        
        void addSchedules(const Arguments& args);
        void removeSchedules(const Arguments& args);
        void changeSchedules(const Arguments& args);
        
    public:
    
        InfoSmeComponent(InfoSmeAdmin& admin);
        virtual ~InfoSmeComponent();
        
        virtual const char* const getName() const {
            return "InfoSme";
        }
        virtual const Methods& getMethods() const {
            return methods;
        }
        
        virtual Variant call(const Method& method, const Arguments& args)
            throw (AdminException);
    };

}}

#endif // ifndef SMSC_INFOSME_COMPONENT
