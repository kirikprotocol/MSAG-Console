
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

    dsOperationTimeout = 0; // in seconds
    try
    {
        dsOperationTimeout = config->getInt("timeout", "Operation timeout "
                                            "on DataSource wasn't defined. "
                                            "Timeout disabled.");
        if (dsOperationTimeout < 0) {
            __trace__("Specified operation timeout "
                       "on DataSource is negative. Timeout disabled.");
            dsOperationTimeout = 0;
        }
    }
    catch (ConfigException& exc) {
        /* Do nothing, dsOperationTimeout = 0 (disabled) */
    }

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

    int wdTimerId = ds.startTimer(connection, dsOperationTimeout);

    try
    {
        Statement* stmt = connection->getStatement(id.c_str(), sql);
        if (!stmt) {
            ds.freeConnection(connection);
            error(SQL_JOB_DS_FAILURE, "Failed to create DataSource statement!");
        }
        process(command, *stmt);             // throws CommandProcessException
        if (!isQuery) connection->commit();  // throws SQLException
    }
    catch(CommandProcessException& exc)
    {
        try{ connection->rollback(); } catch (...) {smsc_log_warn(log,  "Rollback failed");}
        ds.stopTimer(wdTimerId);
        ds.freeConnection(connection);
        throw;
    }
    catch(Exception& exc)
    {
        try{ connection->rollback(); } catch (...) {smsc_log_warn(log,  "Rollback failed");}
        ds.stopTimer(wdTimerId);
        ds.freeConnection(connection);
        error(SQL_JOB_DS_FAILURE, exc.what());
    }
    catch(std::exception& exc)
    {
        try{ connection->rollback(); } catch (...) {smsc_log_warn(log,  "Rollback failed");}
        ds.stopTimer(wdTimerId);
        ds.freeConnection(connection);
        smsc_log_warn(log, "std::exception catched");
        error(SQL_JOB_DS_FAILURE, exc.what());
    }
    catch(...)
    {
        try{ connection->rollback(); } catch (...) {smsc_log_warn(log,  "Rollback failed");}
        ds.stopTimer(wdTimerId);
        ds.freeConnection(connection);
        smsc_log_warn(log, "... catched");
        error(SQL_JOB_DS_FAILURE, "");
    }

    ds.stopTimer(wdTimerId);
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

        try {
            parser->parse(input, setAdapter, ctx);
        } catch (ParsingWarning& wng) {
            smsc_log_warn(log, "%s", wng.what());
        }
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
        try
        {
            std::auto_ptr<ResultSet> rsGuard(stmt.executeQuery());
            ResultSet* rs = rsGuard.get();
            if (!rs)
                error(SQL_JOB_DS_FAILURE, "Result set of query execution is undefined!");

            SQLGetAdapter getAdapter(rs);
            if (!rs->fetchNext())
                error(SQL_JOB_QUERY_NULL, "Result set of query execution is NULL!");

            do formatter->format(output, getAdapter, ctx);
            while (rs->fetchNext());
        }
        catch (FormattingException& exc) {
            error(SQL_JOB_OUTPUT_FORMAT, exc.what());
        }
        catch (CommandProcessException& exc) {
            throw;
        }
        catch (Exception& exc) {
            error(SQL_JOB_DS_FAILURE, exc.what());
        }
    }
    else // Not query
    {
        try
        {
            uint32_t result = (uint32_t)stmt.executeUpdate();
            SQLGetRowsAdapter getAdapter(result);
            formatter->format(output, getAdapter, ctx);
        }
        catch (FormattingException& exc) {
            error(SQL_JOB_OUTPUT_FORMAT, exc.what());
        }
        catch (Exception& exc) {
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

    int wdTimerId = ds.startTimer(connection, dsOperationTimeout);

    try
    {
        Routine* routine = connection->getRoutine(id.c_str(), sql, isFunction);
        if (!routine) {
            ds.freeConnection(connection);
            error(SQL_JOB_DS_FAILURE, "Failed to create DataSource routine call!");
        }
        process(command, *routine);            // throws CommandProcessException
        if (needCommit) connection->commit();  // throws SQLException
    }
    catch(CommandProcessException& exc)
    {
        try{ connection->rollback(); } catch (...) {smsc_log_warn(log,  "Rollback failed");}
        ds.stopTimer(wdTimerId);
        ds.freeConnection(connection);
        throw;
    }
    catch(Exception& exc)
    {
        try{ connection->rollback(); } catch (...) {smsc_log_warn(log,  "Rollback failed");}
        ds.stopTimer(wdTimerId);
        ds.freeConnection(connection);
        error(SQL_JOB_DS_FAILURE, exc.what());
    }
    catch(std::exception& exc)
    {
        try{ connection->rollback(); } catch (...) {smsc_log_warn(log,  "Rollback failed");}
        ds.stopTimer(wdTimerId);
        ds.freeConnection(connection);
        smsc_log_warn(log, "std::exception catched");
        error(SQL_JOB_DS_FAILURE, exc.what());
    }
    catch(...)
    {
        try{ connection->rollback(); } catch (...) {smsc_log_warn(log,  "Rollback failed");}
        ds.stopTimer(wdTimerId);
        ds.freeConnection(connection);
        smsc_log_warn(log, "... catched");
        error(SQL_JOB_DS_FAILURE, "");
    }

    ds.stopTimer(wdTimerId);
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
        try {
            parser->parse(input, routineAdapter, ctx);
        } catch (ParsingWarning& wng) {
            smsc_log_warn(log, "%s", wng.what());
        }
        routine.execute();
        formatter->format(output, routineAdapter, ctx);
    }
    catch (FormattingException& exc) {
        error(SQL_JOB_OUTPUT_FORMAT, exc.what());
    }
    catch (ParsingException& exc) {
        error(SQL_JOB_INPUT_PARSE, exc.what());
    }
    catch (Exception& exc) {
        error(SQL_JOB_DS_FAILURE, exc.what());
    }

    command.setOutData(output.c_str());
}


}}
