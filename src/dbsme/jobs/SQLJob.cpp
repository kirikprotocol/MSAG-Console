
#include "SQLJob.h"
#include "SQLAdapters.h"

namespace smsc { namespace dbsme 
{

using namespace smsc::dbsme::io;

SQLJob::~SQLJob()
{
    if (sql) delete sql;
    if (inputFormat) delete inputFormat;
    if (outputFormat) delete outputFormat;
    
    if (parser) delete parser;
    if (formatter) delete formatter;
}

void SQLJob::init(ConfigView* config)
    throw(ConfigException)
{
    __require__(config);
    
    __trace__("SQLJob loading ... ");
    
    sql = config->getString("sql", "SQL request wasn't defined !");
    inputFormat = config->getString("input", 
                                    "Input data format wasn't defined !");
    outputFormat = config->getString("output", 
                                    "Output data format wasn't defined !");
    isQuery = config->getBool("query", "Type of SQL request undefined !");
    
    try 
    {
        parser = new InputParser(inputFormat);
        formatter = new OutputFormatter(outputFormat);
    }
    catch(Exception& exc)
    {
        throw ConfigException("SQL Job init failed !: %s", exc.what());
    }
    
    __trace__("SQLJob loaded !\n");
}

void SQLJob::process(Command& command, DataSource& ds)
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
            throw CommandProcessException();
        }
    }
    else
    {
        throw CommandProcessException();
    }
    ds.freeConnection(connection);
}

void SQLJob::process(Command& command, Statement& stmt) 
    throw(CommandProcessException)
{
    __trace__("SQL Job: Process command called !!!\n");
    try
    {
        if (!parser || !formatter)
            throw CommandProcessException("IO Parser or Formatter"
                                          " wasn't defined !");

        SQLSetAdapter setAdapter(&stmt);
            
        std::string input = 
            (command.getInData()) ? command.getInData():"";
        parser->parse(input, (SetAdapter&)setAdapter);

        ResultSet* rs = (isQuery) ? stmt.executeQuery() : 0;
        rs->fetchNext();
        
        SQLGetAdapter getAdapter(rs);

        std::string output = "";
        formatter->format(output, (GetAdapter&)getAdapter);
        command.setOutData(output.c_str()); 
    }
    catch(Exception& exc)
    {
        throw CommandProcessException(exc);
    }
}

}}

