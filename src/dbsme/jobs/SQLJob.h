#ifndef SMSC_DBSME_SQLJOB
#define SMSC_DBSME_SQLJOB

/**
 * 
 * @author Victor V. Makarov
 * @version 1.0
 * @see 
 */

#include <util/Logger.h>

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
    
    static const char* SMSC_DBSME_SQL_JOB_IDENTITY  = "sql-job";
    
    static const char* SMSC_DBSME_SQL_JOB_FROM_ADDR = "from-address";
    static const char* SMSC_DBSME_SQL_JOB_TO_ADDR   = "to-address";
    static const char* SMSC_DBSME_SQL_JOB_NAME      = "job-name";

    static const char* SQL_JOB_DS_FAILURE         = "DS_FAILURE";
    static const char* SQL_JOB_DS_CONNECTION_LOST = "DS_CONNECTION_LOST";
    static const char* SQL_JOB_DS_STATEMENT_FAIL  = "DS_STATEMENT_FAIL";

    static const char* SQL_JOB_QUERY_NULL       = "QUERY_NULL";
    static const char* SQL_JOB_INPUT_PARSE      = "INPUT_PARSE";
    static const char* SQL_JOB_OUTPUT_FORMAT    = "OUTPUT_FORMAT";
    static const char* SQL_JOB_INVALID_CONFIG   = "INVALID_CONFIG";
    

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

