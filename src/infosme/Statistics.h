#ifndef SMSC_INFO_SME_STATISTICS
#define SMSC_INFO_SME_STATISTICS

#include <string>

namespace smsc { namespace infosme 
{
    struct TaskStat
    {
        unsigned generated, delivered, retried, failed;
        
        TaskStat(unsigned generated=0, unsigned delivered=0, 
                 unsigned retried=0, unsigned failed=0) 
            : generated(generated), delivered(delivered), 
                retried(retried), failed(failed) {};
        TaskStat(const TaskStat& stat) 
            : generated(stat.generated), delivered(stat.delivered), 
                retried(stat.retried), failed(stat.failed) {};
        
        TaskStat& operator =(const TaskStat& stat) {
            generated = stat.generated; delivered = stat.delivered; 
            retried   = stat.retried;   failed    = stat.failed;
            return (*this);
        };
    };

    class Statistics
    {
    public:

        virtual void flushStatistics() = 0;
        virtual bool getStatistics(std::string taskId, TaskStat& stat) = 0;

        virtual void incGenerated(std::string taskId, unsigned inc=1) = 0;
        virtual void incDelivered(std::string taskId, unsigned inc=1) = 0;
        virtual void incRetried(std::string taskId, unsigned inc=1) = 0;
        virtual void incFailed(std::string taskId, unsigned inc=1) = 0;
        
        virtual ~Statistics() {};
        
    protected:
        
        Statistics() {};
    };

}}

#endif // SMSC_INFO_SME_STATISTICS

