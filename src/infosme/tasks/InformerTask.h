#ifndef SMSC_INFO_SME_INFORMER_TASK
#define SMSC_INFO_SME_INFORMER_TASK

#include <infosme/Task.h>

namespace smsc { namespace infosme
{

    class InformerTask : public Task
    {
    protected:
        
        InformerTask() : Task() {};
    
    public:
        
        virtual ~InformerTask() {};
    };

}}

#endif // SMSC_INFO_SME_INFORMER_TASK
