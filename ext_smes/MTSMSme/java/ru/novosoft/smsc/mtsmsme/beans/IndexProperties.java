package ru.novosoft.smsc.mtsmsme.beans;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.SortedList;
import ru.novosoft.smsc.util.config.Config;

import javax.servlet.http.HttpServletRequest;
import java.util.List;
import java.util.Map;
import java.util.Iterator;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 06.04.2004
 * Time: 15:03:15
 * To change this template use Options | File Templates.
 */
public abstract class IndexProperties extends MTSMSmeBean
{
  public static final String MAPPING_SECTION_NAME = "MTSMSme.Aliases";

  private boolean initialized = false;

  private String smscHost = "";
  private int smscPort = 0;
  private String smscSid = "";
  private int smscTimeout = 0;
  private String smscPassword = "";

  private String mapping_new_address = null;
  private String mapping_new_alias = null;

  private String mbApply = null;
  private String mbReset = null;
  private String mbStart = null;
  private String mbStop = null;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)  return result;

    if (!initialized)
    {
      try
      {
        //smppThreadPoolMax = getConfig().getInt("MTSMSme.SMPPThreadPool.max");
        //smppThreadPoolInit = getConfig().getInt("MTSMSme.SMPPThreadPool.init");

        //adminHost = getConfig().getString("MTSMSme.Admin.host");
        //adminPort = getConfig().getInt("MTSMSme.Admin.port");

        smscHost = getConfig().getString("MTSMSme.SMSC.host");
        smscPort = getConfig().getInt("MTSMSme.SMSC.port");
        smscSid = getConfig().getString("MTSMSme.SMSC.sid");
        smscTimeout = getConfig().getInt("MTSMSme.SMSC.timeout");
        smscPassword = getConfig().getString("MTSMSme.SMSC.password");

      } catch (Exception e) {
        logger.error(e);
        return error(e.getMessage());
      }
    }
    return result;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)  return result;

    try {
      getAppContext().getHostsManager().refreshServices();
    } catch (AdminException e) {
      logger.error("Could not refresh services status", e);
      result = error("Could not refresh services status", e);
    }

    if (initialized) setAliases(request.getParameterMap());

    if (mbApply != null)  return apply();
    if (mbReset != null)  return reset();
    if (mbStart != null)  return start();
    if (mbStop != null)   return stop();

    return result;
  }

  protected int save()
  {
    //getConfig().setInt   ("MTSMSme.SMPPThreadPool.max", smppThreadPoolMax);
    //getConfig().setInt   ("MTSMSme.SMPPThreadPool.init", smppThreadPoolInit);

    //getConfig().setString("MTSMSme.Admin.host", adminHost);
    //getConfig().setInt   ("MTSMSme.Admin.port", adminPort);

    getConfig().setString("MTSMSme.SMSC.host", smscHost);
    getConfig().setInt   ("MTSMSme.SMSC.port", smscPort);
    getConfig().setString("MTSMSme.SMSC.sid", smscSid);
    getConfig().setInt   ("MTSMSme.SMSC.timeout", smscTimeout);
    getConfig().setString("MTSMSme.SMSC.password", smscPassword);

    return RESULT_DONE;
  }

  public String getSmscHost() {
    return smscHost;
  }
  public void setSmscHost(String smscHost) {
    this.smscHost = smscHost;
  }
  public int getSmscPortInt() {
    return smscPort;
  }
  public void setSmscPortInt(int smscPort) {
    this.smscPort = smscPort;
  }
  public String getSmscPort() {
    return String.valueOf(smscPort);
  }
  public void setSmscPort(String smscPort) {
    try {
      this.smscPort = Integer.decode(smscPort).intValue();
    } catch (NumberFormatException e) {
      logger.error("Invalid MTSMSme.SMSC.port parameter value: \"" + smscPort + '"', e);
      this.smscPort = 0;
    }
  }

  public boolean isInitialized() {
    return initialized;
  }
  public void setInitialized(boolean initialized) {
    this.initialized = initialized;
  }

  public String getSmscSid() {
    return smscSid;
  }
  public void setSmscSid(String smscSid) {
    this.smscSid = smscSid;
  }

  public int getSmscTimeoutInt() {
    return smscTimeout;
  }
  public void setSmscTimeoutInt(int smscTimeout) {
    this.smscTimeout = smscTimeout;
  }
  public String getSmscTimeout() {
    return String.valueOf(smscTimeout);
  }
  public void setSmscTimeout(String smscTimeout) {
    try {
      this.smscTimeout = Integer.valueOf(smscTimeout).intValue();
    } catch (NumberFormatException e) {
      logger.error("Invalid MTSMSme.SMSC.timeout parameter value: \"" + smscTimeout + '"', e);
      this.smscTimeout = 0;
    }
  }

  public String getSmscPassword() {
    return smscPassword;
  }
  public void setSmscPassword(String smscPassword) {
    this.smscPassword = smscPassword;
  }

  public List getMappingSectionNames() {
    return new SortedList(getConfig().getSectionChildShortSectionNames(MAPPING_SECTION_NAME));
  }

  public int getInt(String paramName)
      throws Config.ParamNotFoundException, Config.WrongParamTypeException
  {
    return getConfig().getInt(paramName);
  }
  public String getString(String paramName)
      throws Config.ParamNotFoundException, Config.WrongParamTypeException
  {
    return getConfig().getString(paramName);
  }
  public boolean getBool(String paramName)
      throws Config.ParamNotFoundException, Config.WrongParamTypeException
  {
    return getConfig().getBool(paramName);
  }
  private String getParamValue(Object paramObjectValue)
  {
    if (paramObjectValue instanceof String)
      return (String) paramObjectValue;
    else if (paramObjectValue instanceof String[]) {
      String[] paramValues = (String[]) paramObjectValue;
      StringBuffer result = new StringBuffer();
      for (int i = 0; i < paramValues.length; i++) {
        result.append(paramValues[i]);
      }
      return result.toString();
    } else return null;
  }
  private int setAliases(Map requestParams)
  {
    final String PREFIX = MAPPING_SECTION_NAME + '.';

    getConfig().removeSection(MAPPING_SECTION_NAME);
    for (Iterator i = requestParams.keySet().iterator(); i.hasNext();) {
      String paramName = (String) i.next();
      if (paramName.startsWith(PREFIX) && paramName.endsWith("alias")) {
        final String paramValue = getParamValue(requestParams.get(paramName));
        if (paramValue != null)
          getConfig().setString(paramName, paramValue);
      }
    }
    if (mapping_new_address != null && mapping_new_address.length() > 0 &&
        mapping_new_alias != null && mapping_new_alias.length() > 0)
      getConfig().setString(PREFIX + mapping_new_address + ".alias", mapping_new_alias);

    return RESULT_DONE;
  }

  public String getMapping_new_address() {
    return mapping_new_address;
  }
  public void setMapping_new_address(String mapping_new_address) {
    this.mapping_new_address = mapping_new_address;
  }
  public String getMapping_new_alias() {
    return mapping_new_alias;
  }
  public void setMapping_new_alias(String mapping_new_alias) {
    this.mapping_new_alias = mapping_new_alias;
  }

  protected abstract int apply();
  protected abstract int reset();
  protected abstract int start();
  protected abstract int stop();

  public byte getStatus() {
    return getSmeStatus();
  }
  public boolean isMTSMSmeStarted() {
    return isSmeRunning();
  }

  public String getMbApply() {
    return mbApply;
  }
  public void setMbApply(String mbApply) {
    this.mbApply = mbApply;
  }
  public String getMbReset() {
    return mbReset;
  }
  public void setMbReset(String mbReset) {
    this.mbReset = mbReset;
  }
  public String getMbStart() {
    return mbStart;
  }
  public void setMbStart(String mbStart) {
    this.mbStart = mbStart;
  }
  public String getMbStop() {
    return mbStop;
  }
  public void setMbStop(String mbStop) {
    this.mbStop = mbStop;
  }

}
