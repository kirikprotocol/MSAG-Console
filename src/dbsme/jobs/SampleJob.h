#ifndef SMSC_DBSME_SAMPLEJOB
#define SMSC_DBSME_SAMPLEJOB

/**
 * 
 * @author Victor V. Makarov
 * @version 1.0
 * @see 
 */

#include <dbsme/Jobs.h>

namespace smsc { namespace dbsme 
{
    using namespace smsc::core::buffers;
    using namespace smsc::db;
    
    using smsc::util::Logger;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;

    class SampleJob : public Job
    {
    public:

        SampleJob() : Job() {};
        virtual ~SampleJob() {};
        
        virtual void init(ConfigView* config)
            throw(ConfigException);
        
        virtual void process(Command& command, Statement& stmt) 
            throw(CommandProcessException);
    };

    class SampleJobFactory : public JobFactory
    {
    protected:

        virtual Job* createJob() 
        {
            return new SampleJob();
        };
        
    public:
        
        SampleJobFactory() : JobFactory() {};
        virtual ~SampleJobFactory() {};
    };

}}

#endif

