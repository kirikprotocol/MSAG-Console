
#include "SQLJob.h"
#include "SQLAdapters.h"

namespace smsc { namespace dbsme 
{

using namespace smsc::util::templates;
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
    //__trace__("SQLJob loading ... ");
    
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
    
    //__trace__("SQLJob loaded !\n");
}

void SQLJob::process(Command& command, DataSource& ds)
    throw(CommandProcessException)
{
    Connection* connection = ds.getConnection();
    if (!connection) error(SQL_JOB_DS_FAILURE, 
                           "Failed to create DataSource connection!");
    Statement* stmt = 0;
    try 
    {
        stmt = connection->createStatement(sql); // throws SQLException
        if (!stmt) 
        {
            ds.freeConnection(connection);
            error(SQL_JOB_DS_FAILURE, 
                  "Failed to create DataSource statement!");
        }
        process(command, *stmt);             // throws CommandProcessException
        if (!isQuery) connection->commit();  // throws SQLException
        delete stmt;
    }
    catch(CommandProcessException& exc) 
    {
        if (stmt) delete stmt;
        connection->rollback();
        ds.freeConnection(connection);
        throw;
    }
    catch(Exception& exc)
    {
        if (stmt) delete stmt;
        connection->rollback();
        ds.freeConnection(connection);
        error(SQL_JOB_DS_FAILURE, exc.what());
    }

    ds.freeConnection(connection);
}

void SQLJob::process(Command& command, Statement& stmt) 
    throw(CommandProcessException)
{
    ContextEnvironment ctx;
    ctx.exportStr(SMSC_DBSME_SQL_JOB_FROM_ADDR, command.getFromAddress().value);
    ctx.exportStr(SMSC_DBSME_SQL_JOB_TO_ADDR, command.getToAddress().value);
    ctx.exportStr(SMSC_DBSME_SQL_JOB_NAME, getName());

    command.setOutData("");
    
    if (!parser || !formatter) 
        error(SQL_JOB_INVALID_CONFIG, 
              "IO Parser or Formatter wasn't defined!");
    
    std::string input = (command.getInData()) ? command.getInData():"";

    try 
    {
        SQLSetAdapter setAdapter(&stmt);
        parser->parse(input, setAdapter, ctx); 
    }
    catch (ParsingException& exc) 
    {
        error(SQL_JOB_INPUT_PARSE, exc.what());
    }
    catch (Exception& exc) 
    {
        error(SQL_JOB_DS_FAILURE, exc.what());
    }

    std::string output = "";
    if (isQuery)
    {
        ResultSet* rs = 0;
        try 
        {
            rs = stmt.executeQuery();
            if (!rs)
                error(SQL_JOB_DS_FAILURE, 
                      "Result set of query execution is undefined!");
            
            SQLGetAdapter getAdapter(rs);
            if (!rs->fetchNext()) 
                error(SQL_JOB_QUERY_NULL, 
                      "Result set of query execution is NULL!");
            
            do formatter->format(output, getAdapter, ctx);
            while (rs->fetchNext());
        } 
        catch (FormattingException& exc)
        {
            error(SQL_JOB_OUTPUT_FORMAT, exc.what());
        }
        catch (CommandProcessException& exc) 
        {
            if (rs) delete rs;
            throw;
        }
        catch (Exception& exc) 
        {
            if (rs) delete rs;
            error(SQL_JOB_DS_FAILURE, exc.what());
        }
        if (rs) delete rs;
    }
    else // Not query
    {
        try
        {
            uint32_t result = (uint32_t)stmt.executeUpdate();
            SQLGetRowsAdapter getAdapter(result);
            formatter->format(output, getAdapter, ctx);
        }
        catch (FormattingException& exc)
        {
            error(SQL_JOB_OUTPUT_FORMAT, exc.what());
        }
        catch (Exception& exc) 
        {
            error(SQL_JOB_DS_FAILURE, exc.what());
        }
    }
    
    command.setOutData(output.c_str()); 
}

}}

