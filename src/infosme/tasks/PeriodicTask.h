#ifndef SMSC_INFO_SME_PERIODIC_TASK
#define SMSC_INFO_SME_PERIODIC_TASK

#include "InformerTask.h"

namespace smsc { namespace infosme
{

    static const char* INFO_SME_PERIODIC_TASK_IDENTITY = "periodic";
    
    class PeriodicTask : public InformerTask
    {
    public:
    
        PeriodicTask() : InformerTask() {};
        virtual ~PeriodicTask() {};
    };

    class PeriodicTaskFactory : public TaskFactory
    {
    protected:

        virtual Task* createTask() 
        {
            //return new PeriodicTask();
            return 0;
        };
        
    public:
        
        PeriodicTaskFactory() : TaskFactory() {};
        virtual ~PeriodicTaskFactory() {};
    };

}}

#endif // SMSC_INFO_SME_PERIODIC_TASK
