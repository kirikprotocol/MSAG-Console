package ru.novosoft.smsc.dbsme;

import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.Filter;
import ru.novosoft.smsc.jsp.util.tables.EmptyResultSet;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataItem;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSourceImpl;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractQueryImpl;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.SortedList;

import java.security.Principal;
import java.util.*;

/**
 * Created by igork
 * Date: Jul 11, 2003
 * Time: 4:48:42 PM
 */
public class Provider extends DbsmeBean {
  public static final int RESULT_EDIT = DbsmeBean.PRIVATE_RESULT;
  public static final int RESULT_ADD = DbsmeBean.PRIVATE_RESULT + 1;
  public static final int RESULT_REFRESH = DbsmeBean.PRIVATE_RESULT + 2;
  public static final int PRIVATE_RESULT = DbsmeBean.PRIVATE_RESULT + 3;

  private boolean creating = false;
  private boolean initialized = false;
  private String providerName = null;
  private String oldProviderName = null;
  private String jobName = null;
  private String prefix;
  private String address = null;
  private int connections = 0;
  private String dbInstance = null;
  private String dbUserName = null;
  private String dbUserPassword = null;
  private String type = null;
  private boolean watchdog = false;
  private String job_not_found = null;
  private String ds_failure = null;
  private String ds_connection_lost = null;
  private String ds_statement_fail = null;
  private String query_null = null;
  private String input_parse = null;
  private String output_format = null;
  private String invalid_config = null;

  private QueryResultSet jobs = null;
  private Set checkedSet = new HashSet();
  private String[] checked = new String[0];
  private String mbEdit = null;
  private String mbDone = null;
  private String mbCancel = null;
  private String mbAdd = null;
  private String mbDelete = null;

  private class _DataItem extends AbstractDataItem {
    protected _DataItem(String name, String type, String address, String alias)
    {
      values.put("name", name);
      values.put("type", type);
      values.put("address", address);
      values.put("alias", alias);
    }
  }

  private class _DataSource extends AbstractDataSourceImpl {

    public _DataSource()
    {
      super(new String[]{"name", "type", "address", "alias"});
    }

    public QueryResultSet query(_Query query_to_run)
    {
      clear();
      for (Iterator i = getSectionChildSectionNames(prefix + ".Jobs").iterator(); i.hasNext();) {
        String fullJobName = (String) i.next();
        String jobName = getString(fullJobName + ".name");
        String jobType = getString(fullJobName + ".type");
        String jobAddress = getString(fullJobName + ".address");
        String jobAlias = getString(fullJobName + ".alias");
        add(new _DataItem(jobName, jobType, jobAddress, jobAlias));
      }
      return super.query(query_to_run);
    }
  }

  private class _Query extends AbstractQueryImpl {
    public _Query(int expectedResultsQuantity, Filter filter, Vector sortOrder, int startPosition)
    {
      super(expectedResultsQuantity, filter, sortOrder, startPosition);
    }
  }

  private class _Filter implements Filter {
    public boolean isEmpty()
    {
      return true;
    }

    public boolean isItemAllowed(DataItem item)
    {
      return true;
    }
  }


  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK) {
      jobs = new EmptyResultSet();
      prefix = "";
      if (sort == null)
        sort = "";
      if (pageSize == 0)
        pageSize = 20;
      return result;
    }

    pageSize = 20;
    if (sort != null) {
      //preferences.getAliasesSortOrder().set(0, sort);
    } else {
      //sort = (String) preferences.getAliasesSortOrder().get(0);
      sort = "name";
    }

    if (providerName == null || providerName.length() == 0)
      if (creating)
        providerName = "";
      else
        return error(DBSmeErrors.error.provider.providerNotSpecified);

    if (!initialized) {
      if (creating) {
        providerName = "";
        oldProviderName = "";
        prefix = null;
        address = "";
        connections = 0;
        dbInstance = "";
        dbUserName = "";
        dbUserPassword = "";
        type = "";
        watchdog = false;

        job_not_found = "";
        ds_failure = "";
        ds_connection_lost = "";
        ds_statement_fail = "";
        query_null = "";
        input_parse = "";
        output_format = "";
        invalid_config = "";
      } else {
        oldProviderName = providerName;
        prefix = "DBSme.DataProviders." + StringEncoderDecoder.encodeDot(oldProviderName);
        address = getString(prefix + ".address");
        connections = getInt(prefix + ".DataSource.connections");
        dbInstance = getString(prefix + ".DataSource.dbInstance");
        dbUserName = getString(prefix + ".DataSource.dbUserName");
        dbUserPassword = getString(prefix + ".DataSource.dbUserPassword");
        type = getString(prefix + ".DataSource.type");
        watchdog = getBool(prefix + ".DataSource.watchdog");

        job_not_found = getString(prefix + ".MessageSet.JOB_NOT_FOUND");
        ds_failure = getString(prefix + ".MessageSet.DS_FAILURE");
        ds_connection_lost = getString(prefix + ".MessageSet.DS_CONNECTION_LOST");
        ds_statement_fail = getString(prefix + ".MessageSet.DS_STATEMENT_FAIL");
        query_null = getString(prefix + ".MessageSet.QUERY_NULL");
        input_parse = getString(prefix + ".MessageSet.INPUT_PARSE");
        output_format = getString(prefix + ".MessageSet.OUTPUT_FORMAT");
        invalid_config = getString(prefix + ".MessageSet.INVALID_CONFIG");
      }
    }
    if (oldProviderName == null) oldProviderName = "";
    prefix = "DBSme.DataProviders." + StringEncoderDecoder.encodeDot(oldProviderName);

    if (prefix == null) prefix = "";
    if (address == null) address = "";
    if (dbInstance == null) dbInstance = "";
    if (dbUserName == null) dbUserName = "";
    if (dbUserPassword == null) dbUserPassword = "";
    if (type == null) type = "";

    if (job_not_found == null) job_not_found = "";
    if (ds_failure == null) ds_failure = "";
    if (ds_connection_lost == null) ds_connection_lost = "";
    if (ds_statement_fail == null) ds_statement_fail = "";
    if (query_null == null) query_null = "";
    if (input_parse == null) input_parse = "";
    if (output_format == null) output_format = "";
    if (invalid_config == null) invalid_config = "";

    return result;
  }

  public int process(SMSCAppContext appContext, List errors, Principal loginedPrincipal)
  {
    int result = super.process(appContext, errors, loginedPrincipal);
    if (result != RESULT_OK)
      return result;

    if (mbEdit != null && mbEdit.length() > 0 && providerName != null && providerName.length() > 0)
      return RESULT_EDIT;

    if (mbDelete != null)
      result = delete();
    if (result != RESULT_OK)
      return result;

    Vector sortVector = new Vector();
    sortVector.add(sort);
    jobs = new _DataSource().query(new _Query(pageSize, new _Filter(), sortVector, startPosition));
    totalSize = jobs.getTotalSize();

    if (mbCancel != null)
      return RESULT_DONE;
    if (mbDone != null)
      return save();
    if (mbAdd != null) {
      result = save();
      if (result != RESULT_DONE)
        return result;
      else
        return RESULT_ADD;
    }

    checkedSet.addAll(Arrays.asList(checked));

    return result;
  }

  private int delete()
  {
    for (int i = 0; i < checked.length; i++)
      config.removeSection(prefix + ".Jobs." + checked[i]);

    int result = save();
    if (result != RESULT_DONE)
      return result;

    DbSmeContext.getInstance(appContext).setConfigChanged(true);

    checked = new String[0];
    checkedSet.clear();
    return RESULT_REFRESH;
  }

  private int save()
  {
    String newPrefix = "DBSme.DataProviders." + StringEncoderDecoder.encodeDot(providerName);
    if (!creating) {
      if (!newPrefix.equals(prefix)) {
        final String tmp = getString(newPrefix + ".type");
        if (tmp != null && tmp.length() > 0)
          return error(DBSmeErrors.error.provider.providerAlreadyExists, providerName);
        config.renameSection(prefix, newPrefix);
      }
    }

    config.setString(newPrefix + ".address", address);
    config.setString(newPrefix + ".DataSource.type", type);
    config.setInt(newPrefix + ".DataSource.connections", connections);
    config.setString(newPrefix + ".DataSource.dbInstance", dbInstance);
    config.setString(newPrefix + ".DataSource.dbUserName", dbUserName);
    config.setString(newPrefix + ".DataSource.dbUserPassword", dbUserPassword);

    if (watchdog)
      config.setBool(newPrefix + ".DataSource.watchdog", watchdog);
    else
      config.removeParam(newPrefix + ".DataSource.watchdog");

    if (job_not_found != null && job_not_found.length() > 0) config.setString(newPrefix + ".MessageSet.JOB_NOT_FOUND", job_not_found); else config.removeParam(newPrefix + ".MessageSet.JOB_NOT_FOUND");
    if (ds_failure != null && ds_failure.length() > 0) config.setString(newPrefix + ".MessageSet.DS_FAILURE", ds_failure); else config.removeParam(newPrefix + ".MessageSet.DS_FAILURE");
    if (ds_connection_lost != null && ds_connection_lost.length() > 0) config.setString(newPrefix + ".MessageSet.DS_CONNECTION_LOST", ds_connection_lost); else config.removeParam(newPrefix + ".MessageSet.DS_CONNECTION_LOST");
    if (ds_statement_fail != null && ds_statement_fail.length() > 0) config.setString(newPrefix + ".MessageSet.DS_STATEMENT_FAIL", ds_statement_fail); else config.removeParam(newPrefix + ".MessageSet.DS_STATEMENT_FAIL");
    if (query_null != null && query_null.length() > 0) config.setString(newPrefix + ".MessageSet.QUERY_NULL", query_null); else config.removeParam(newPrefix + ".MessageSet.QUERY_NULL");
    if (input_parse != null && input_parse.length() > 0) config.setString(newPrefix + ".MessageSet.INPUT_PARSE", input_parse); else config.removeParam(newPrefix + ".MessageSet.INPUT_PARSE");
    if (output_format != null && output_format.length() > 0) config.setString(newPrefix + ".MessageSet.OUTPUT_FORMAT", output_format); else config.removeParam(newPrefix + ".MessageSet.OUTPUT_FORMAT");
    if (invalid_config != null && invalid_config.length() > 0) config.setString(newPrefix + ".MessageSet.INVALID_CONFIG", invalid_config); else config.removeParam(newPrefix + ".MessageSet.INVALID_CONFIG");

    try {
      config.save();
    } catch (Exception e) {
      logger.error("Couldn't save temporary config, nested: " + e.getMessage(), e);
      return error(DBSmeErrors.error.couldntSaveTempConfig, e);
    }

    DbSmeContext.getInstance(appContext).setConfigChanged(true);

    return RESULT_DONE;
  }

  public List getTypes()
  {
    List result = new SortedList();
    for (Iterator i = getSectionChildSectionNames("StartupLoader.DataSourceDrivers").iterator(); i.hasNext();) {
      String driverSection = (String) i.next();
      result.add(getString(driverSection + ".type"));
    }
    return result;
  }

  public String getPrefix()
  {
    return prefix;
  }

  public boolean isJobChecked(String jobName)
  {
    return checkedSet.contains(jobName);
  }

  public QueryResultSet getJobs()
  {
    return jobs;
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

  public String getAddress()
  {
    return address;
  }

  public void setAddress(String address)
  {
    this.address = address;
  }

  public String getConnections()
  {
    return String.valueOf(connections);
  }

  public void setConnections(String connections)
  {
    try {
      this.connections = Integer.decode(connections).intValue();
    } catch (NumberFormatException e) {
      this.connections = 0;
    }
  }

  public int getConnectionsInt()
  {
    return connections;
  }

  public void setConnectionsInt(int connections)
  {
    this.connections = connections;
  }

  public String getDbInstance()
  {
    return dbInstance;
  }

  public void setDbInstance(String dbInstance)
  {
    this.dbInstance = dbInstance;
  }

  public String getDbUserName()
  {
    return dbUserName;
  }

  public void setDbUserName(String dbUserName)
  {
    this.dbUserName = dbUserName;
  }

  public String getDbUserPassword()
  {
    return dbUserPassword;
  }

  public void setDbUserPassword(String dbUserPassword)
  {
    this.dbUserPassword = dbUserPassword;
  }

  public String getType()
  {
    return type;
  }

  public void setType(String type)
  {
    this.type = type;
  }

  public String getMbEdit()
  {
    return mbEdit;
  }

  public void setMbEdit(String mbEdit)
  {
    this.mbEdit = mbEdit;
  }

  public String getMbDone()
  {
    return mbDone;
  }

  public void setMbDone(String mbDone)
  {
    this.mbDone = mbDone;
  }

  public String getMbCancel()
  {
    return mbCancel;
  }

  public void setMbCancel(String mbCancel)
  {
    this.mbCancel = mbCancel;
  }

  public String getOldProviderName()
  {
    return oldProviderName;
  }

  public void setOldProviderName(String oldProviderName)
  {
    this.oldProviderName = oldProviderName;
  }

  public boolean isWatchdog()
  {
    return watchdog;
  }

  public void setWatchdog(boolean watchdog)
  {
    this.watchdog = watchdog;
  }

  public String getJob_not_found()
  {
    return job_not_found;
  }

  public void setJob_not_found(String job_not_found)
  {
    this.job_not_found = job_not_found;
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

  public String getMbAdd()
  {
    return mbAdd;
  }

  public void setMbAdd(String mbAdd)
  {
    this.mbAdd = mbAdd;
  }

  public String getMbDelete()
  {
    return mbDelete;
  }

  public void setMbDelete(String mbDelete)
  {
    this.mbDelete = mbDelete;
  }

  public String[] getChecked()
  {
    return checked;
  }

  public void setChecked(String[] checked)
  {
    this.checked = checked;
  }
}
