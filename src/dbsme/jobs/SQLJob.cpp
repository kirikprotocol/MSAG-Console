
#include "SQLJob.h"
#include "SQLAdapters.h"

namespace smsc { namespace dbsme 
{

const int MAX_FULL_ADDRESS_VALUE_LENGTH = 30;
typedef char FullAddressValue[MAX_FULL_ADDRESS_VALUE_LENGTH];

using namespace smsc::util::templates;
using namespace smsc::dbsme::io;

SQLQueryJob::~SQLQueryJob()
{
    if (sql) delete sql;
    if (inputFormat) delete inputFormat;
    if (outputFormat) delete outputFormat;
    
    if (parser) delete parser;
    if (formatter) delete formatter;
}

void SQLQueryJob::init(ConfigView* config)
    throw(ConfigException)
{
    __require__(config);
    
    sql = config->getString("sql", "SQL query block wasn't defined !");
    inputFormat = config->getString("input", 
                                    "Input data format wasn't defined !");
    outputFormat = config->getString("output", 
                                    "Output data format wasn't defined !");
    try 
    {
        parser = new InputParser(inputFormat);
        formatter = new OutputFormatter(outputFormat);
    }
    catch(Exception& exc)
    {
        throw ConfigException("SQLQueryJob init failed !: %s", exc.what());
    }
}

void SQLJob::init(ConfigView* config)
    throw(ConfigException)
{
    SQLQueryJob::init(config);
    isQuery = config->getBool("query", "Type of SQL request undefined !");
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
        try{ connection->rollback(); } catch (...) {log.warn( "Rollback failed");}
        ds.freeConnection(connection);
        throw;
    }
    catch(Exception& exc)
    {
        if (stmt) delete stmt;
        try{ connection->rollback(); } catch (...) {log.warn( "Rollback failed");}
        ds.freeConnection(connection);
        error(SQL_JOB_DS_FAILURE, exc.what());
    }
    catch(std::exception& exc) 
    {
        if (stmt) delete stmt;
        try{ connection->rollback(); } catch (...) {log.warn( "Rollback failed");}
        ds.freeConnection(connection);
        log.warn("std::exception catched");
        error(SQL_JOB_DS_FAILURE, exc.what());
    }
    catch(...) 
    {
        if (stmt) delete stmt;
        try{ connection->rollback(); } catch (...) {log.warn( "Rollback failed");}
        ds.freeConnection(connection);
        log.warn("... catched");
        error(SQL_JOB_DS_FAILURE, "");
    }

    ds.freeConnection(connection);
}

void SQLJob::process(Command& command, Statement& stmt) 
    throw(CommandProcessException)
{
    ContextEnvironment ctx;
    FullAddressValue fromAddress, toAddress;
    FullAddressValue fromAddressUn, toAddressUn;
    command.getFromAddress().getText(fromAddress, sizeof(fromAddress));
    command.getToAddress().getText(toAddress, sizeof(toAddress));
    command.getFromAddress().toString(fromAddressUn, sizeof(fromAddressUn));
    command.getToAddress().toString(toAddressUn, sizeof(toAddressUn));
    ctx.exportStr(SMSC_DBSME_SQL_JOB_FROM_ADDR, fromAddress);
    ctx.exportStr(SMSC_DBSME_SQL_JOB_TO_ADDR, toAddress);
    ctx.exportStr(SMSC_DBSME_SQL_JOB_FROM_ADDR_UN, fromAddressUn);
    ctx.exportStr(SMSC_DBSME_SQL_JOB_TO_ADDR_UN, toAddressUn);
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
            if (rs) delete rs;
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

void PLSQLJob::init(ConfigView* config)
    throw(ConfigException)
{
    SQLQueryJob::init(config);
    isFunction = config->getBool("function", 
                                 "Type of PL/SQL call undefined "
                                 "(Function or Procedure)");
    needCommit = config->getBool("commit",
                                 "Mode for PL/SQL call undefined "
                                 "(Commit or No commit)");
}

void PLSQLJob::process(Command& command, DataSource& ds)
    throw(CommandProcessException)
{
    Connection* connection = ds.getConnection();
    if (!connection) error(SQL_JOB_DS_FAILURE, 
                           "Failed to create DataSource connection!");
    Routine* routine = 0;
    try 
    {
        routine = connection->createRoutine(sql, isFunction); 
        if (!routine) 
        {
            ds.freeConnection(connection);
            error(SQL_JOB_DS_FAILURE, 
                  "Failed to create DataSource routine call!");
        }
        process(command, *routine);            // throws CommandProcessException
        if (needCommit) connection->commit();  // throws SQLException
        delete routine;
    }
    catch(CommandProcessException& exc) 
    {
        if (routine) delete routine;
        try{ connection->rollback(); } catch (...) {log.warn( "Rollback failed");}
        ds.freeConnection(connection);
        throw;
    }
    catch(Exception& exc)
    {
        if (routine) delete routine;
        try{ connection->rollback(); } catch (...) {log.warn( "Rollback failed");}
        ds.freeConnection(connection);
        error(SQL_JOB_DS_FAILURE, exc.what());
    }
    catch(std::exception& exc) 
    {
        if (routine) delete routine;
        try{ connection->rollback(); } catch (...) {log.warn( "Rollback failed");}
        ds.freeConnection(connection);
        log.warn("std::exception catched");
        error(SQL_JOB_DS_FAILURE, exc.what());
    }
    catch(...) 
    {
        if (routine) delete routine;
        try{ connection->rollback(); } catch (...) {log.warn( "Rollback failed");}
        ds.freeConnection(connection);
        log.warn("... catched");
        error(SQL_JOB_DS_FAILURE, "");
    }

    ds.freeConnection(connection);
}

void PLSQLJob::process(Command& command, Routine& routine) 
    throw(CommandProcessException)
{
    ContextEnvironment ctx;
    FullAddressValue fromAddress, toAddress;
    FullAddressValue fromAddressUn, toAddressUn;
    command.getFromAddress().getText(fromAddress, sizeof(fromAddress));
    command.getToAddress().getText(toAddress, sizeof(toAddress));
    command.getFromAddress().toString(fromAddressUn, sizeof(fromAddressUn));
    command.getToAddress().toString(toAddressUn, sizeof(toAddressUn));
    ctx.exportStr(SMSC_DBSME_SQL_JOB_FROM_ADDR, fromAddress);
    ctx.exportStr(SMSC_DBSME_SQL_JOB_TO_ADDR, toAddress);
    ctx.exportStr(SMSC_DBSME_SQL_JOB_FROM_ADDR_UN, fromAddressUn);
    ctx.exportStr(SMSC_DBSME_SQL_JOB_TO_ADDR_UN, toAddressUn);
    ctx.exportStr(SMSC_DBSME_SQL_JOB_NAME, getName());

    command.setOutData("");
    
    if (!parser || !formatter) 
        error(SQL_JOB_INVALID_CONFIG, 
              "IO Parser or Formatter wasn't defined!");
    
    std::string input = (command.getInData()) ? command.getInData():"";
    std::string output = "";
    
    try 
    {
        SQLRoutineAdapter routineAdapter(&routine);
        parser->parse(input, routineAdapter, ctx); 
        routine.execute();
        formatter->format(output, routineAdapter, ctx);
    }
    catch (FormattingException& exc)
    {
        error(SQL_JOB_OUTPUT_FORMAT, exc.what());
    }
    catch (ParsingException& exc) 
    {
        error(SQL_JOB_INPUT_PARSE, exc.what());
    }
    catch (Exception& exc) 
    {
        error(SQL_JOB_DS_FAILURE, exc.what());
    }
    
    command.setOutData(output.c_str()); 
}


}}

