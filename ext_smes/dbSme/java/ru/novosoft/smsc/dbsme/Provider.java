package ru.novosoft.smsc.dbsme;

import ru.novosoft.smsc.jsp.util.tables.*;
import ru.novosoft.smsc.jsp.util.tables.impl.*;
import ru.novosoft.smsc.util.SortedList;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

/**
 * Created by igork
 * Date: Jul 11, 2003
 * Time: 4:48:42 PM
 */
public class Provider extends DbsmeBean
{
  public static final int RESULT_EDIT = DbsmeBean.PRIVATE_RESULT;
  public static final int RESULT_ADD = DbsmeBean.PRIVATE_RESULT + 1;
  public static final int RESULT_REFRESH = DbsmeBean.PRIVATE_RESULT + 2;
  public static final int PRIVATE_RESULT = DbsmeBean.PRIVATE_RESULT + 3;

  private boolean creating = false;
  private boolean initialized = false;
  private String providerName = "";
  private String oldProviderName = "";
  private String prefix;
  private String address = "";
  private int connections = 0;
  private String dbInstance = "";
  private String dbUserName = "";
  private String dbUserPassword = "";
  private String type = "";
  private boolean watchdog = false;
  private boolean enabled = false;
  private String service_not_available = "";
  private String job_not_found = "";
  private String ds_failure = "";
  private String ds_connection_lost = "";
  private String ds_statement_fail = "";
  private String query_null = "";
  private String input_parse = "";
  private String output_format = "";
  private String invalid_config = "";

  private QueryResultSet jobs = new EmptyResultSet();
  private Set checkedSet = new HashSet();
  private String[] checked = new String[0];
  private String mbEdit = null;
  private String mbDone = null;
  private String mbCancel = null;
  private String mbAdd = null;
  private String mbDelete = null;
  private String edit = null;

  private class _DataItem extends AbstractDataItem
  {
    protected _DataItem(String id, String name, String type, String address, String alias)
    {
      values.put("id", id);
      values.put("name", name);
      values.put("type", type);
      values.put("address", address);
      values.put("alias", alias);
    }
  }

  private class _DataSource extends AbstractDataSourceImpl
  {

    public _DataSource()
    {
      super(new String[]{"id", "name", "type", "address", "alias"});
    }

    public QueryResultSet query(_Query query_to_run)
    {
      clear();
      final String jobsSection = prefix + ".Jobs";
      for (Iterator i = getSectionChildSectionNames(jobsSection).iterator(); i.hasNext();) {
        String fullJobName = (String) i.next();
        String jobId = fullJobName.substring(jobsSection.length() + 1);
        String jobName = getOptionalString(fullJobName + ".name");
        String jobType = getString(fullJobName + ".type");
        String jobAddress = getOptionalString(fullJobName + ".address");
        String jobAlias = getOptionalString(fullJobName + ".alias");
        add(new _DataItem(jobId, jobName, jobType, jobAddress, jobAlias));
      }
      return super.query(query_to_run);
    }
  }

  private class _Query extends AbstractQueryImpl
  {
    public _Query(int expectedResultsQuantity, Filter filter, Vector sortOrder, int startPosition)
    {
      super(expectedResultsQuantity, filter, sortOrder, startPosition);
    }
  }

  private class _Filter implements Filter
  {
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
        return error("dbsme.error.no_provider");

    if (!initialized) {
      if (creating) {
        providerName = "";
        oldProviderName = "";
        prefix = null;
        enabled = false;
        address = "";
        connections = 0;
        dbInstance = "";
        dbUserName = "";
        dbUserPassword = "";
        type = "";
        watchdog = false;

        service_not_available = "";
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
        enabled = getOptionalBool(prefix + ".enabled");
        address = getString(prefix + ".address");
        connections = getInt(prefix + ".DataSource.connections");
        dbInstance = getString(prefix + ".DataSource.dbInstance");
        dbUserName = getString(prefix + ".DataSource.dbUserName");
        dbUserPassword = getString(prefix + ".DataSource.dbUserPassword");
        type = getString(prefix + ".DataSource.type");
        watchdog = getOptionalBool(prefix + ".DataSource.watchdog");

        service_not_available = getOptionalString(prefix + ".MessageSet.SERVICE_NOT_AVAIL");
        job_not_found = getOptionalString(prefix + ".MessageSet.JOB_NOT_FOUND");
        ds_failure = getOptionalString(prefix + ".MessageSet.DS_FAILURE");
        ds_connection_lost = getOptionalString(prefix + ".MessageSet.DS_CONNECTION_LOST");
        ds_statement_fail = getOptionalString(prefix + ".MessageSet.DS_STATEMENT_FAIL");
        query_null = getOptionalString(prefix + ".MessageSet.QUERY_NULL");
        input_parse = getOptionalString(prefix + ".MessageSet.INPUT_PARSE");
        output_format = getOptionalString(prefix + ".MessageSet.OUTPUT_FORMAT");
        invalid_config = getOptionalString(prefix + ".MessageSet.INVALID_CONFIG");
      }
    }
    if (oldProviderName == null) oldProviderName = "";
    prefix = "DBSme.DataProviders." + StringEncoderDecoder.encodeDot(oldProviderName);

    if (prefix == null) prefix = "";
    if (providerName == null) providerName = "";
    if (address == null) address = "";
    if (dbInstance == null) dbInstance = "";
    if (dbUserName == null) dbUserName = "";
    if (dbUserPassword == null) dbUserPassword = "";
    if (type == null) type = "";

    if (service_not_available == null) service_not_available = "";
    if (job_not_found == null) job_not_found = "";
    if (ds_failure == null) ds_failure = "";
    if (ds_connection_lost == null) ds_connection_lost = "";
    if (ds_statement_fail == null) ds_statement_fail = "";
    if (query_null == null) query_null = "";
    if (input_parse == null) input_parse = "";
    if (output_format == null) output_format = "";
    if (invalid_config == null) invalid_config = "";

    providerName = providerName.trim();

    return result;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (mbCancel != null)
      return RESULT_DONE;
    if (mbDone != null)
      return save(false);
    if (mbAdd != null) {
      result = save(false);
      if (result != RESULT_DONE)
        return result;
      else
        return RESULT_ADD;
    }
    if (mbEdit != null && mbEdit.length() > 0 && providerName != null && providerName.length() > 0) {
      result = save(false);
      if (result != RESULT_DONE)
        return result;
      else
        return RESULT_EDIT;
    }
    if (mbDelete != null) {
      result = delete();
      if (result != RESULT_OK)
        return result;
    }

    Vector sortVector = new Vector();
    sortVector.add(sort);
    jobs = new _DataSource().query(new _Query(pageSize, new _Filter(), sortVector, startPosition));
    totalSize = jobs.getTotalSize();

    checkedSet.addAll(Arrays.asList(checked));

    return result;
  }

  private int delete()
  {
    logger.debug("Delete: " + checked.length);
    for (int i = 0; i < checked.length; i++) {
      config.removeSection(prefix + ".Jobs." + checked[i]);
      logger.debug("  remove sec: " + prefix + ".Jobs." + checked[i]);
    }

    getContext().setJobsChanged(true);

    int result = save(true);
    if (result != RESULT_DONE)
      return result;

    checked = new String[0];
    checkedSet.clear();

    return RESULT_REFRESH;
  }

  private int save(boolean forceSave)
  {
    final String newPrefix = createProviderPrefix(providerName);
    final boolean providerEquals = isProviderEquals(providerName, oldProviderName, address, connections, dbInstance, dbUserName, dbUserPassword, type, watchdog,
                                                    service_not_available, job_not_found, ds_failure, ds_connection_lost, ds_statement_fail, query_null, input_parse, output_format, invalid_config);
    final boolean enabledEquals = enabled == getOptionalBool(newPrefix + ".enabled");

    if (!providerEquals) {
      if (creating) {
        if (config.containsSection(newPrefix))
          return error("dbsme.error.exist_provider", providerName);
      } else {
        if (!providerName.equals(oldProviderName)) {
          final String oldProviderPrefix = createProviderPrefix(oldProviderName);
          if (config.containsSection(oldProviderPrefix))
            return error("dbsme.error.exist_provider", providerName);
          config.renameSection(oldProviderPrefix, newPrefix);
        }
      }
      setProviderParamsToConfig(newPrefix);
      try {
        config.save();
      } catch (Exception e) {
        logger.error("Couldn't save temporary config, nested: " + e.getMessage(), e);
        return error("dbsme.error.config_save", e);
      }
      getContext().setConfigChanged(true);
      return RESULT_DONE;
    } else {
      if (forceSave) {
        try {
          config.save();
        } catch (Exception e) {
          logger.error("Couldn't save temporary config, nested: " + e.getMessage(), e);
          return error("dbsme.error.config_save", e);
        }
      }
      if (!enabledEquals) {
        if (getContext().isConfigChanged()) {
          return error(enabled ? "dbsme.error.provider_enable" : "dbsme.error.provider_disable", providerName);
        }
        int result = setProviderEnabled(providerName, enabled);
        if (result != RESULT_DONE)
          return result;
        try {
          config.save();
        } catch (Exception e) {
          logger.error("Couldn't save temporary config, nested: " + e.getMessage(), e);
          return error("dbsme.error.config_save", e);
        }
      }
      return RESULT_DONE;
    }
  }

  private void setProviderParamsToConfig(final String providerPrefix)
  {
    config.setBool(providerPrefix + ".enabled", enabled);
    config.setString(providerPrefix + ".address", address);
    config.setString(providerPrefix + ".DataSource.type", type);
    config.setInt(providerPrefix + ".DataSource.connections", connections);
    config.setString(providerPrefix + ".DataSource.dbInstance", dbInstance);
    config.setString(providerPrefix + ".DataSource.dbUserName", dbUserName);
    config.setString(providerPrefix + ".DataSource.dbUserPassword", dbUserPassword);

    if (watchdog)
      config.setBool(providerPrefix + ".DataSource.watchdog", watchdog);
    else
      config.removeParam(providerPrefix + ".DataSource.watchdog");

    if (service_not_available != null && service_not_available.length() > 0) config.setString(providerPrefix + ".MessageSet.SERVICE_NOT_AVAIL", service_not_available); else config.removeParam(providerPrefix + ".MessageSet.SERVICE_NOT_AVAIL");
    if (job_not_found != null && job_not_found.length() > 0) config.setString(providerPrefix + ".MessageSet.JOB_NOT_FOUND", job_not_found); else config.removeParam(providerPrefix + ".MessageSet.JOB_NOT_FOUND");
    if (ds_failure != null && ds_failure.length() > 0) config.setString(providerPrefix + ".MessageSet.DS_FAILURE", ds_failure); else config.removeParam(providerPrefix + ".MessageSet.DS_FAILURE");
    if (ds_connection_lost != null && ds_connection_lost.length() > 0) config.setString(providerPrefix + ".MessageSet.DS_CONNECTION_LOST", ds_connection_lost); else config.removeParam(providerPrefix + ".MessageSet.DS_CONNECTION_LOST");
    if (ds_statement_fail != null && ds_statement_fail.length() > 0) config.setString(providerPrefix + ".MessageSet.DS_STATEMENT_FAIL", ds_statement_fail); else config.removeParam(providerPrefix + ".MessageSet.DS_STATEMENT_FAIL");
    if (query_null != null && query_null.length() > 0) config.setString(providerPrefix + ".MessageSet.QUERY_NULL", query_null); else config.removeParam(providerPrefix + ".MessageSet.QUERY_NULL");
    if (input_parse != null && input_parse.length() > 0) config.setString(providerPrefix + ".MessageSet.INPUT_PARSE", input_parse); else config.removeParam(providerPrefix + ".MessageSet.INPUT_PARSE");
    if (output_format != null && output_format.length() > 0) config.setString(providerPrefix + ".MessageSet.OUTPUT_FORMAT", output_format); else config.removeParam(providerPrefix + ".MessageSet.OUTPUT_FORMAT");
    if (invalid_config != null && invalid_config.length() > 0) config.setString(providerPrefix + ".MessageSet.INVALID_CONFIG", invalid_config); else config.removeParam(providerPrefix + ".MessageSet.INVALID_CONFIG");
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

  public String getEdit()
  {
    return edit;
  }

  public void setEdit(String edit)
  {
    this.edit = edit;
  }

  public boolean isEnabled()
  {
    return enabled;
  }

  public void setEnabled(boolean enabled)
  {
    this.enabled = enabled;
  }

  public Set getCheckedSet()
  {
    return checkedSet;
  }

  public void setCheckedSet(Set checkedSet)
  {
    this.checkedSet = checkedSet;
  }

  public String getService_not_available()
  {
    return service_not_available;
  }

  public void setService_not_available(String service_not_available)
  {
    this.service_not_available = service_not_available;
  }
}
