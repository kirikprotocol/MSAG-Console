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
            flushStatisticsMethod, addTasksMethod, removeTasksMethod, 
            startTasksMethod, stopTasksMethod, setTaskEnabledMethod,
            addScheduleMethod, removeScheduleMethod, changeScheduleMethod
        };
        
        void error(const char* method, const char* param);
    
    protected:
        
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
