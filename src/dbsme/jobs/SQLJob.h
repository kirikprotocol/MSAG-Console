#ifndef SMSC_DBSME_SQLJOB
#define SMSC_DBSME_SQLJOB

/**
 * 
 * @author Victor V. Makarov
 * @version 1.0
 * @see 
 */

#include <dbsme/Jobs.h>
#include <dbsme/io/Parsers.h>
#include <dbsme/io/Formatters.h>

namespace smsc { namespace dbsme 
{
    using namespace smsc::core::buffers;
    using namespace smsc::dbsme::io;
    using namespace smsc::db;
    
    using smsc::util::Logger;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;
    
    class SQLJob : public Job
    {
    protected:

        InputParser*        parser;
        OutputFormatter*    formatter;

    public:

        SQLJob() : Job(), parser(0), formatter(0) {};
        virtual ~SQLJob() 
        {
            if (parser) delete parser;
            if (formatter) delete formatter;
        };
        
        virtual void init(ConfigView* config)
            throw(ConfigException);
        
        virtual void process(Command& command, Statement& stmt) 
            throw(CommandProcessException);
    };

    class SQLJobFactory : public JobFactory
    {
    protected:

        virtual Job* createJob() 
        {
            return new SQLJob();
        };
        
    public:
        
        SQLJobFactory() : JobFactory() {};
        virtual ~SQLJobFactory() {};
    };

}}

#endif

