#ifndef SMSC_INFO_SME_DISTRIBUTE_TASK
#define SMSC_INFO_SME_DISTRIBUTE_TASK

#include "InformerTask.h"

namespace smsc { namespace infosme
{

    static const char* INFO_SME_DISTRIBUTE_TASK_IDENTITY = "distribute";
    
    class DistributeTask : public InformerTask
    {
    public:
    
        DistributeTask() : InformerTask() {};
        virtual ~DistributeTask() {};
    };

    class DistributeTaskFactory : public TaskFactory
    {
    protected:

        virtual Task* createTask() 
        {
            //return new DistributeTask();
            return 0;
        };
        
    public:
        
        DistributeTaskFactory() : TaskFactory() {};
        virtual ~DistributeTaskFactory() {};
    };

}}

#endif // SMSC_INFO_SME_DISTRIBUTE_TASK
