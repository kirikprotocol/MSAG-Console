#ifndef SMSC_DBSME_SQLJOB
#define SMSC_DBSME_SQLJOB

/**
 * 
 * @author Victor V. Makarov
 * @version 1.0
 * @see 
 */

#include <dbsme/Jobs.h>
#include <util/templates/Parsers.h>
#include <util/templates/Formatters.h>

namespace smsc { namespace dbsme 
{
    using namespace smsc::util::templates;
    using namespace smsc::core::buffers;
    using namespace smsc::db;
    
    using smsc::util::Logger;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;
    
    static const char* SMSC_DBSME_SQL_JOB_IDENTITY = "sql-job";

    class SQLJob : public Job
    {
    protected:

        InputParser*        parser;
        OutputFormatter*    formatter;

        char*   sql;
        char*   inputFormat;
        char*   outputFormat;
        
        bool    isQuery;

        virtual void process(Command& command, Statement& stmt) 
            throw(CommandProcessException);
        
    public:

        SQLJob() : Job(), parser(0), formatter(0),
            sql(0), inputFormat(0), outputFormat(0) {};

        virtual ~SQLJob();
        
        virtual void init(ConfigView* config)
            throw(ConfigException);
        virtual void process(Command& command, DataSource& ds)
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

