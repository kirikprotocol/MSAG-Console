package ru.novosoft.smsc.dbsme;

import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import java.security.Principal;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * Created by igork
 * Date: Jul 14, 2003
 * Time: 3:27:02 PM
 */
public class Job extends DbsmeBean
{
  public static final String TYPE_SQL = "sql-job";
  public static final String TYPE_PLSQL = "pl/sql-job";

  private String providerName = null;
  private String jobName = null;
  private String jobId = null;
  private String type = null;
  private String address = null;
  private String alias = null;
  private int timeout = 0;
  private boolean query = false;		//SQL
  private boolean commit = false;		//PL_SQL
  private boolean function = false;	//PL_SQL
  private String sql = null;
  private String input = null;
  private String output = null;
  private String ds_failure = null;
  private String ds_connection_lost = null;
  private String ds_statement_fail = null;
  private String query_null = null;
  private String input_parse = null;
  private String output_format = null;
  private String invalid_config = null;

  private boolean providerWatchdog = false;
  private String mbCancel = null;
  private String mbDone = null;
  private boolean creating = false;
  private boolean initialized = false;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK) {
      timeout = 0;
      type = address = alias = sql = input = output = "";
      query = commit = function = false;
      return result;
    }

    if (jobName == null)
      jobName = "";

    if (!initialized) {
      if (creating) {
        jobId = "";
        type = "";
        address = "";
        alias = "";
        timeout = 0;
        sql = "";
        input = "";
        output = "";
        query = false;
        commit = false;
        function = false;
        ds_failure = "";
        ds_connection_lost = "";
        ds_statement_fail = "";
        query_null = "";
        input_parse = "";
        output_format = "";
        invalid_config = "";
      } else {
        if (jobId == null)
          return error(DBSmeErrors.error.job.jobNotFound);

        String jobPrefix = createJobPrefix(providerName, jobId);

        jobName = getOptionalString(jobPrefix + ".name");
        type = getString(jobPrefix + ".type");
        address = getOptionalString(jobPrefix + ".address");
        alias = getOptionalString(jobPrefix + ".alias");
        timeout = getInt(jobPrefix + ".timeout");
        sql = getString(jobPrefix + ".sql");
        input = getString(jobPrefix + ".input");
        output = getString(jobPrefix + ".output");
        query = getBool(jobPrefix + ".query");
        commit = getBool(jobPrefix + ".commit");
        function = getBool(jobPrefix + ".function");
        ds_failure = getOptionalString(jobPrefix + ".MessageSet.DS_FAILURE");
        ds_connection_lost = getOptionalString(jobPrefix + ".MessageSet.DS_CONNECTION_LOST");
        ds_statement_fail = getOptionalString(jobPrefix + ".MessageSet.DS_STATEMENT_FAIL");
        query_null = getOptionalString(jobPrefix + ".MessageSet.QUERY_NULL");
        input_parse = getOptionalString(jobPrefix + ".MessageSet.INPUT_PARSE");
        output_format = getOptionalString(jobPrefix + ".MessageSet.OUTPUT_FORMAT");
        invalid_config = getOptionalString(jobPrefix + ".MessageSet.INVALID_CONFIG");
      }
    }
    if (type == null) type = "";
    if (address == null) address = "";
    if (alias == null) alias = "";
    if (sql == null) sql = "";
    if (input == null) input = "";
    if (output == null) output = "";
    if (ds_failure == null) ds_failure = "";
    if (ds_connection_lost == null) ds_connection_lost = "";
    if (ds_statement_fail == null) ds_statement_fail = "";
    if (query_null == null) query_null = "";
    if (input_parse == null) input_parse = "";
    if (output_format == null) output_format = "";
    if (invalid_config == null) invalid_config = "";

    providerWatchdog = getBool("DBSme.DataProviders." + StringEncoderDecoder.encodeDot(providerName) + ".DataSource.watchdog");
    return result;
  }

  protected static String createJobPrefix(String providerName, String jobId)
  {
    return Provider.createProviderPrefix(providerName) + ".Jobs." + StringEncoderDecoder.encodeDot(jobId);
  }

  public int process(SMSCAppContext appContext, List errors, Principal loginedPrincipal)
  {
    int result = super.process(appContext, errors, loginedPrincipal);
    if (result != RESULT_OK)
      return result;

    if (mbCancel != null)
      return RESULT_DONE;
    if (mbDone != null)
      return save();

    return result;
  }

  private int save()
  {
    if (creating) {
      jobId = jobName;
      if (jobId == null || jobId.length() == 0) {
        SimpleDateFormat format = new SimpleDateFormat("yyyy.MM.dd.HH.mm.ss.SSS");
        jobId = "Job " + format.format(new Date());
      }
    }
    String jobPrefix = createJobPrefix(providerName, jobId);

    if (creating) {
      String jobIdPrefix = jobId + " ";
      for (int counter = 1; config.containsSection(jobPrefix); counter++) {
        jobId = jobIdPrefix + counter;
        jobPrefix = createJobPrefix(providerName, jobId);
      }
    }

    if (!creating)
      config.removeSection(jobPrefix);

    config.setString(jobPrefix + ".name", jobName);
    config.setString(jobPrefix + ".type", type);
    config.setString(jobPrefix + ".sql", sql);
    config.setString(jobPrefix + ".input", input);
    config.setString(jobPrefix + ".output", output);

    if (address != null && address.length() > 0) config.setString(jobPrefix + ".address", address);
    if (alias != null && alias.length() > 0) config.setString(jobPrefix + ".alias", alias);
    if (providerWatchdog) config.setInt(jobPrefix + ".timeout", timeout);
    if (TYPE_SQL.equals(type))
      config.setBool(jobPrefix + ".query", query);
    else if (TYPE_PLSQL.equals(type)) {
      config.setBool(jobPrefix + ".commit", commit);
      config.setBool(jobPrefix + ".function", function);
    } else {
      return error(DBSmeErrors.error.job.unknownType, type);
    }

    if (ds_failure != null && ds_failure.length() > 0) config.setString(jobPrefix + ".MessageSet.DS_FAILURE", ds_failure);
    if (ds_connection_lost != null && ds_connection_lost.length() > 0) config.setString(jobPrefix + ".MessageSet.DS_CONNECTION_LOST", ds_connection_lost);
    if (ds_statement_fail != null && ds_statement_fail.length() > 0) config.setString(jobPrefix + ".MessageSet.DS_STATEMENT_FAIL", ds_statement_fail);
    if (query_null != null && query_null.length() > 0) config.setString(jobPrefix + ".MessageSet.QUERY_NULL", query_null);
    if (input_parse != null && input_parse.length() > 0) config.setString(jobPrefix + ".MessageSet.INPUT_PARSE", input_parse);
    if (output_format != null && output_format.length() > 0) config.setString(jobPrefix + ".MessageSet.OUTPUT_FORMAT", output_format);
    if (invalid_config != null && invalid_config.length() > 0) config.setString(jobPrefix + ".MessageSet.INVALID_CONFIG", invalid_config);

    try {
      config.save();
    } catch (Exception e) {
      logger.error("Couldn't save temporary config, nested: " + e.getMessage(), e);
      return error(DBSmeErrors.error.couldntSaveTempConfig, e);
    }

    getContext().setJobsChanged(true);
    return RESULT_DONE;
  }

  public String getProviderName()
  {
    return providerName;
  }

  public void setProviderName(String providerName)
  {
    this.providerName = providerName;
  }

  public String getJobName()
  {
    return jobName;
  }

  public void setJobName(String jobName)
  {
    this.jobName = jobName;
  }

  public String getType()
  {
    return type;
  }

  public void setType(String type)
  {
    this.type = type;
  }

  public String getAddress()
  {
    return address;
  }

  public void setAddress(String address)
  {
    this.address = address;
  }

  public String getAlias()
  {
    return alias;
  }

  public void setAlias(String alias)
  {
    this.alias = alias;
  }

  public String getTimeout()
  {
    return String.valueOf(timeout);
  }

  public void setTimeout(String timeout)
  {
    try {
      this.timeout = Integer.decode(timeout).intValue();
    } catch (NumberFormatException e) {
      logger.debug("Couldn't understand timeout, nested: " + e.getMessage(), e);
      this.timeout = 0;
    }
  }

  public int getTimeoutInt()
  {
    return timeout;
  }

  public void setTimeoutInt(int timeout)
  {
    this.timeout = timeout;
  }

  public boolean isQuery()
  {
    return query;
  }

  public void setQuery(boolean query)
  {
    this.query = query;
  }

  public boolean isCommit()
  {
    return commit;
  }

  public void setCommit(boolean commit)
  {
    this.commit = commit;
  }

  public boolean isFunction()
  {
    return function;
  }

  public void setFunction(boolean function)
  {
    this.function = function;
  }

  public String getSql()
  {
    return sql;
  }

  public void setSql(String sql)
  {
    this.sql = sql;
  }

  public String getInput()
  {
    return input;
  }

  public void setInput(String input)
  {
    this.input = input;
  }

  public String getOutput()
  {
    return output;
  }

  public void setOutput(String output)
  {
    this.output = output;
  }

  public boolean isProviderWatchdog()
  {
    return providerWatchdog;
  }

  public String getMbCancel()
  {
    return mbCancel;
  }

  public void setMbCancel(String mbCancel)
  {
    this.mbCancel = mbCancel;
  }

  public String getMbDone()
  {
    return mbDone;
  }

  public void setMbDone(String mbDone)
  {
    this.mbDone = mbDone;
  }

  public String getDs_failure()
  {
    return ds_failure;
  }

  public void setDs_failure(String ds_failure)
  {
    this.ds_failure = ds_failure;
  }

  public String getDs_connection_lost()
  {
    return ds_connection_lost;
  }

  public void setDs_connection_lost(String ds_connection_lost)
  {
    this.ds_connection_lost = ds_connection_lost;
  }

  public String getDs_statement_fail()
  {
    return ds_statement_fail;
  }

  public void setDs_statement_fail(String ds_statement_fail)
  {
    this.ds_statement_fail = ds_statement_fail;
  }

  public String getQuery_null()
  {
    return query_null;
  }

  public void setQuery_null(String query_null)
  {
    this.query_null = query_null;
  }

  public String getInput_parse()
  {
    return input_parse;
  }

  public void setInput_parse(String input_parse)
  {
    this.input_parse = input_parse;
  }

  public String getOutput_format()
  {
    return output_format;
  }

  public void setOutput_format(String output_format)
  {
    this.output_format = output_format;
  }

  public String getInvalid_config()
  {
    return invalid_config;
  }

  public void setInvalid_config(String invalid_config)
  {
    this.invalid_config = invalid_config;
  }

  public boolean isCreating()
  {
    return creating;
  }

  public void setCreating(boolean creating)
  {
    this.creating = creating;
  }

  public boolean isInitialized()
  {
    return initialized;
  }

  public void setInitialized(boolean initialized)
  {
    this.initialized = initialized;
  }

  public String getJobId()
  {
    return jobId;
  }

  public void setJobId(String jobId)
  {
    this.jobId = jobId;
  }
}
