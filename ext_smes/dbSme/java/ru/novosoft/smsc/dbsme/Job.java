package ru.novosoft.smsc.dbsme;

import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.config.Config;

import java.security.Principal;
import java.util.Iterator;
import java.util.List;

/**
 * Created by igork
 * Date: Jul 14, 2003
 * Time: 3:27:02 PM
 */
public class Job extends DbsmeBean {
	public static final String TYPE_SQL = "sql-job";
	public static final String TYPE_PLSQL = "pl/sql-job";

	private String providerName = null;
	private String jobName = null;
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

	private String jobPrefix = null;
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
				jobPrefix = null;
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
				jobPrefix = findJobPrefix(providerName, jobName);
				if (jobPrefix == null || jobPrefix.length() == 0)
					return error(DBSmeErrors.error.job.jobNotFound, jobName);

				type = getString(jobPrefix + ".type");
				address = getString(jobPrefix + ".address");
				alias = getString(jobPrefix + ".alias");
				timeout = getInt(jobPrefix + ".timeout");
				sql = getString(jobPrefix + ".sql");
				input = getString(jobPrefix + ".input");
				output = getString(jobPrefix + ".output");
				query = getBool(jobPrefix + ".query");
				commit = getBool(jobPrefix + ".commit");
				function = getBool(jobPrefix + ".function");
				ds_failure = getString(jobPrefix + ".MessageSet.DS_FAILURE");
				ds_connection_lost = getString(jobPrefix + ".MessageSet.DS_CONNECTION_LOST");
				ds_statement_fail = getString(jobPrefix + ".MessageSet.DS_STATEMENT_FAIL");
				query_null = getString(jobPrefix + ".MessageSet.QUERY_NULL");
				input_parse = getString(jobPrefix + ".MessageSet.INPUT_PARSE");
				output_format = getString(jobPrefix + ".MessageSet.OUTPUT_FORMAT");
				invalid_config = getString(jobPrefix + ".MessageSet.INVALID_CONFIG");
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

	private String findJobPrefix(String providerName, String jobName)
	{
		if (providerName == null || providerName.length() == 0 || jobName == null || jobName.length() == 0)
			return null;
		for (Iterator i = config.getSectionChildSectionNames("DBSme.DataProviders." + StringEncoderDecoder.encodeDot(providerName) + ".Jobs").iterator(); i.hasNext();) {
			String sectionFullName = (String) i.next();
			try {
				if (config.getString(sectionFullName + ".name").equals(jobName))
					return sectionFullName;
			} catch (Config.ParamNotFoundException e) {
				// skip this empty section
			} catch (Config.WrongParamTypeException e) {
				// skip this misformatted section
			}
		}
		return null;
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
		if ((jobPrefix == null || jobPrefix.length() == 0) && !creating)
			return error(DBSmeErrors.error.job.jobNotFound, jobName);

		String newJobPrefix = findJobPrefix(providerName, jobName);
		if (newJobPrefix != null && !newJobPrefix.equals(jobPrefix))
			return error(DBSmeErrors.error.job.jobAlreadyExists, jobName);

		if (!creating)
			config.removeSection(jobPrefix);

		newJobPrefix = "DBSme.DataProviders." + StringEncoderDecoder.encodeDot(providerName) + ".Jobs." + StringEncoderDecoder.encodeDot(jobName);
		config.setString(newJobPrefix + ".name", jobName);
		config.setString(newJobPrefix + ".type", type);
		config.setString(newJobPrefix + ".sql", sql);
		config.setString(newJobPrefix + ".input", input);
		config.setString(newJobPrefix + ".output", output);

		if (address != null && address.length() > 0) config.setString(newJobPrefix + ".address", address);
		if (alias != null && alias.length() > 0) config.setString(newJobPrefix + ".alias", alias);
		if (providerWatchdog) config.setInt(newJobPrefix + ".timeout", timeout);
		if (TYPE_SQL.equals(type))
			config.setBool(newJobPrefix + ".query", query);
		else if (TYPE_PLSQL.equals(type)) {
			config.setBool(newJobPrefix + ".commit", commit);
			config.setBool(newJobPrefix + ".function", function);
		} else {
			return error(DBSmeErrors.error.job.unknownType, type);
		}

		if (ds_failure != null && ds_failure.length() > 0) config.setString(newJobPrefix + ".MessageSet.DS_FAILURE", ds_failure);
		if (ds_connection_lost != null && ds_connection_lost.length() > 0) config.setString(newJobPrefix + ".MessageSet.DS_CONNECTION_LOST", ds_connection_lost);
		if (ds_statement_fail != null && ds_statement_fail.length() > 0) config.setString(newJobPrefix + ".MessageSet.DS_STATEMENT_FAIL", ds_statement_fail);
		if (query_null != null && query_null.length() > 0) config.setString(newJobPrefix + ".MessageSet.QUERY_NULL", query_null);
		if (input_parse != null && input_parse.length() > 0) config.setString(newJobPrefix + ".MessageSet.INPUT_PARSE", input_parse);
		if (output_format != null && output_format.length() > 0) config.setString(newJobPrefix + ".MessageSet.OUTPUT_FORMAT", output_format);
		if (invalid_config != null && invalid_config.length() > 0) config.setString(newJobPrefix + ".MessageSet.INVALID_CONFIG", invalid_config);

		try {
			config.save();
		} catch (Exception e) {
			logger.error("Couldn't save temporary config, nested: " + e.getMessage(), e);
			return error(DBSmeErrors.error.couldntSaveTempConfig, e);
		}

		DbSmeContext.getInstance(appContext).setConfigChanged(true);

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

	public String getJobPrefix()
	{
		return jobPrefix == null ? "" : jobPrefix;
	}

	public void setJobPrefix(String jobPrefix)
	{
		this.jobPrefix = jobPrefix;
	}
}
