
#include "SQLJob.h"
#include "SQLAdapters.h"

namespace smsc { namespace dbsme 
{

using namespace smsc::dbsme::io;

void SQLJob::init(ConfigView* config)
    throw(ConfigException)
{
    Job::init(config);
    
    printf("SQLJob loading ... ");

    try 
    {
        parser = new InputParser(inputFormat);
        formatter = new OutputFormatter(outputFormat);
    }
    catch(Exception& exc)
    {
        throw ConfigException("SQL Job init failed !: %s", exc.what());
    }
    
    printf("SQLJob loaded !\n");
}

void SQLJob::process(Command& command, Statement& stmt) 
    throw(CommandProcessException)
{
    printf("SQL Job: Process command called !!!\n");
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

