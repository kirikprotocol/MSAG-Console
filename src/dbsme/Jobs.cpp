
#include "Jobs.h"

namespace smsc { namespace dbsme 
{

Hash<JobFactory *>*  JobFactory::registry = 0;

/* ------------------------- Command Processing (Job) ---------------------- */
        
void Job::init(ConfigView* config)
    throw(ConfigException)
{
    __require__(config);

    sql = config->getString("sql", "SQL request wasn't defined !");
    inputFormat = config->getString("input", 
                                    "Input data format wasn't defined !");
    outputFormat = config->getString("output", 
                                    "Output data format wasn't defined !");
    isQuery = config->getBool("query", "Type of SQL request undefined !");
}

Job::~Job()
{
    if (sql) delete sql;
    if (inputFormat) delete inputFormat;
    if (outputFormat) delete outputFormat;
}

void Job::process(Command& command, DataSource& ds)
    throw(CommandProcessException)
{
    Connection* connection = ds.getConnection();
    if (connection)
    {
        try 
        {
            Statement* stmt = connection->createStatement(sql);
            if (stmt)
            {
                process(command, *stmt);
                delete stmt;
            }
            else
            {
                ds.freeConnection(connection);
                throw CommandProcessException();
            }
        }
        catch(SQLException& exc)
        {
            ds.freeConnection(connection);
            throw;
        }
    }
    else
    {
        throw CommandProcessException();
    }
    ds.freeConnection(connection);
}

}}
