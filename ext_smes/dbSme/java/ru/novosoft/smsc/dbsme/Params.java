package ru.novosoft.smsc.dbsme;

import ru.novosoft.smsc.jsp.SMSCAppContext;

import javax.servlet.http.HttpServletRequest;
import java.security.Principal;
import java.util.*;

/**
 * Created by igork
 * Date: Jul 10, 2003
 * Time: 6:42:18 PM
 */
public class Params extends DbsmeBean
{
  private String mbDone = null;
  private String mbCancel = null;

  private String svcType = null;
  private int protocolId = 0;
  private String origAddress = null;
  private String systemType = null;
  private int max = 0;
  private int init = 0;
  private String host = null;
  private int port = 0;
  private String sid = null;
  private int timeout = 0;
  private String password = null;
  private String adminHost = null;
  private int adminPort = 0;

  private String provider_not_found = null;
  private String job_not_found = null;
  private String ds_failure = null;
  private String ds_connection_lost = null;
  private String ds_statement_fail = null;
  private String query_null = null;
  private String input_parse = null;
  private String output_format = null;
  private String invalid_config = null;

  private boolean initialized = false;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    if (!initialized) {
      svcType = getString("DBSme.SvcType");
      protocolId = getInt("DBSme.ProtocolId");
      origAddress = getString("DBSme.origAddress");
      systemType = getString("DBSme.systemType");
      max = getInt("DBSme.ThreadPool.max");
      init = getInt("DBSme.ThreadPool.init");
      host = getString("DBSme.SMSC.host");
      port = getInt("DBSme.SMSC.port");
      sid = getString("DBSme.SMSC.sid");
      timeout = getInt("DBSme.SMSC.timeout");
      password = getString("DBSme.SMSC.password");

      adminHost = getString("DBSme.Admin.host");
      adminPort = getInt("DBSme.Admin.port");

      provider_not_found = getOptionalString("DBSme.MessageSet.PROVIDER_NOT_FOUND");
      job_not_found = getOptionalString("DBSme.MessageSet.JOB_NOT_FOUND");
      ds_failure = getOptionalString("DBSme.MessageSet.DS_FAILURE");
      ds_connection_lost = getOptionalString("DBSme.MessageSet.DS_CONNECTION_LOST");
      ds_statement_fail = getOptionalString("DBSme.MessageSet.DS_STATEMENT_FAIL");
      query_null = getOptionalString("DBSme.MessageSet.QUERY_NULL");
      input_parse = getOptionalString("DBSme.MessageSet.INPUT_PARSE");
      output_format = getOptionalString("DBSme.MessageSet.OUTPUT_FORMAT");
      invalid_config = getOptionalString("DBSme.MessageSet.INVALID_CONFIG");
    }
    return result;
  }

  public int process(SMSCAppContext appContext, List errors, Principal loginedPrincipal, HttpServletRequest request)
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
    config.setString("DBSme.SvcType", svcType == null ? "" : svcType);
    config.setInt("DBSme.ProtocolId", protocolId);
    config.setString("DBSme.origAddress", origAddress == null ? "" : origAddress);
    config.setString("DBSme.systemType", systemType == null ? "" : systemType);
    config.setInt("DBSme.ThreadPool.max", max);
    config.setInt("DBSme.ThreadPool.init", init);
    config.setString("DBSme.SMSC.host", host == null ? "" : host);
    config.setInt("DBSme.SMSC.port", port);
    config.setString("DBSme.SMSC.sid", sid == null ? "" : sid);
    config.setInt("DBSme.SMSC.timeout", timeout);
    config.setString("DBSme.SMSC.password", password == null ? "" : password);
    config.setString("DBSme.Admin.host", adminHost);
    config.setInt("DBSme.Admin.port", adminPort);

    if (provider_not_found != null && provider_not_found.length() > 0) config.setString("DBSme.MessageSet.PROVIDER_NOT_FOUND", provider_not_found);
    if (job_not_found != null && job_not_found.length() > 0) config.setString("DBSme.MessageSet.JOB_NOT_FOUND", job_not_found);
    if (ds_failure != null && ds_failure.length() > 0) config.setString("DBSme.MessageSet.DS_FAILURE", ds_failure);
    if (ds_connection_lost != null && ds_connection_lost.length() > 0) config.setString("DBSme.MessageSet.DS_CONNECTION_LOST", ds_connection_lost);
    if (ds_statement_fail != null && ds_statement_fail.length() > 0) config.setString("DBSme.MessageSet.DS_STATEMENT_FAIL", ds_statement_fail);
    if (query_null != null && query_null.length() > 0) config.setString("DBSme.MessageSet.QUERY_NULL", query_null);
    if (input_parse != null && input_parse.length() > 0) config.setString("DBSme.MessageSet.INPUT_PARSE", input_parse);
    if (output_format != null && output_format.length() > 0) config.setString("DBSme.MessageSet.OUTPUT_FORMAT", output_format);
    if (invalid_config != null && invalid_config.length() > 0) config.setString("DBSme.MessageSet.INVALID_CONFIG", invalid_config);

    try {
      config.save();
    } catch (Exception e) {
      logger.error("Couldn't save temporary config, nested: " + e.getMessage(), e);
      return error(DBSmeErrors.error.couldntSaveTempConfig, e);
    }

    getContext().setConfigChanged(true);

    return RESULT_DONE;
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

  public String getSvcType()
  {
    return svcType;
  }

  public void setSvcType(String svcType)
  {
    this.svcType = svcType;
  }

  public int getProtocolIdInt()
  {
    return protocolId;
  }

  public void setProtocolIdInt(int protocolId)
  {
    this.protocolId = protocolId;
  }

  public String getProtocolId()
  {
    return String.valueOf(protocolId);
  }

  public void setProtocolId(String protocolId)
  {
    try {
      this.protocolId = Integer.decode(protocolId).intValue();
    } catch (NumberFormatException e) {
      this.protocolId = 0;
    }
  }

  public String getOrigAddress()
  {
    return origAddress;
  }

  public void setOrigAddress(String origAddress)
  {
    this.origAddress = origAddress;
  }

  public String getSystemType()
  {
    return systemType;
  }

  public void setSystemType(String systemType)
  {
    this.systemType = systemType;
  }

  public int getMaxInt()
  {
    return max;
  }

  public void setMaxInt(int max)
  {
    this.max = max;
  }

  public String getMax()
  {
    return String.valueOf(max);
  }

  public void setMax(String max)
  {
    try {
      this.max = Integer.decode(max).intValue();
    } catch (NumberFormatException e) {
      this.max = 0;
    }
  }

  public int getInitInt()
  {
    return init;
  }

  public void setInitInt(int init)
  {
    this.init = init;
  }

  public String getInit()
  {
    return String.valueOf(init);
  }

  public void setInit(String init)
  {
    try {
      this.init = Integer.decode(init).intValue();
    } catch (NumberFormatException e) {
      this.init = 0;
    }
  }

  public String getHost()
  {
    return host;
  }

  public void setHost(String host)
  {
    this.host = host;
  }

  public int getPortInt()
  {
    return port;
  }

  public void setPortInt(int port)
  {
    this.port = port;
  }

  public String getPort()
  {
    return String.valueOf(port);
  }

  public void setPort(String port)
  {
    try {
      this.port = Integer.decode(port).intValue();
    } catch (NumberFormatException e) {
      this.port = 0;
    }
  }

  public String getSid()
  {
    return sid;
  }

  public void setSid(String sid)
  {
    this.sid = sid;
  }

  public int getTimeoutInt()
  {
    return timeout;
  }

  public void setTimeoutInt(int timeout)
  {
    this.timeout = timeout;
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
      this.timeout = 0;
    }
  }

  public String getPassword()
  {
    return password;
  }

  public void setPassword(String password)
  {
    this.password = password;
  }

  public String getProvider_not_found()
  {
    return provider_not_found;
  }

  public void setProvider_not_found(String provider_not_found)
  {
    this.provider_not_found = provider_not_found;
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

  public boolean isInitialized()
  {
    return initialized;
  }

  public void setInitialized(boolean initialized)
  {
    this.initialized = initialized;
  }

  public String getAdminHost()
  {
    return adminHost;
  }

  public void setAdminHost(String adminHost)
  {
    this.adminHost = adminHost;
  }

  public int getAdminPortInt()
  {
    return adminPort;
  }

  public void setAdminPortInt(int adminPort)
  {
    this.adminPort = adminPort;
  }

  public String getAdminPort()
  {
    return String.valueOf(adminPort);
  }

  public void setAdminPort(String adminPort)
  {
    try {
      this.adminPort = Integer.decode(adminPort).intValue();
    } catch (NumberFormatException e) {
      this.adminPort = 0;
    }
  }
}
