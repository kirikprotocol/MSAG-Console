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
  private int    smscPort = 0;
  private String smscSid = "";
  private int    smscTimeout = 0;
  private String smscPassword = "";

  private int sccpUserId = 0;
  private int sccpUserSsn = 0;
  private String sccpMscGt = "";
  private String sccpVlrGt = "";
  private String sccpHlrGt = "";

  private String mapping_new_address = null;
  private String mapping_new_comment = null;
  private String mapping_new_reg_type = null;
  private String mapping_new_alias = null;
  private String mapping_new_mgt = null;
  private String mapping_new_msisdn = null;
  private int mapping_new_period = -1;

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
      result = reload();
    }

    return result;
  }

  protected int reload() {
    try
      {
        smscHost = getConfig().getString("MTSMSme.SMSC.host");
        smscPort = getConfig().getInt("MTSMSme.SMSC.port");
        smscSid = getConfig().getString("MTSMSme.SMSC.sid");
        smscTimeout = getConfig().getInt("MTSMSme.SMSC.timeout");
        smscPassword = getConfig().getString("MTSMSme.SMSC.password");

        sccpUserId = getConfig().getInt("MTSMSme.SCCP.user_id");
        sccpUserSsn = getConfig().getInt("MTSMSme.SCCP.user_ssn");
        sccpMscGt = getConfig().getString("MTSMSme.SCCP.msc_gt");
        sccpVlrGt = getConfig().getString("MTSMSme.SCCP.vlr_gt");
        sccpHlrGt = getConfig().getString("MTSMSme.SCCP.hlr_gt");

      } catch (Exception e) {
        logger.error(e);
        return error("mtsmsme.error.config_load_failed", e);
      }
    return RESULT_OK;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)  return result;

    try {
      getAppContext().getHostsManager().refreshServices();
    } catch (AdminException e) {
      logger.error("Could not refresh services status", e);
      result = error("mtsmsme.error.refresh_status", e);
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
    getConfig().setString("MTSMSme.SMSC.host", smscHost);
    getConfig().setInt   ("MTSMSme.SMSC.port", smscPort);
    getConfig().setString("MTSMSme.SMSC.sid", smscSid);
    getConfig().setInt   ("MTSMSme.SMSC.timeout", smscTimeout);
    getConfig().setString("MTSMSme.SMSC.password", smscPassword);

    getConfig().setInt("MTSMSme.SCCP.user_id", sccpUserId);
    getConfig().setInt("MTSMSme.SCCP.user_ssn", sccpUserSsn);
    getConfig().setString("MTSMSme.SCCP.msc_gt", sccpMscGt);
    getConfig().setString("MTSMSme.SCCP.vlr_gt", sccpVlrGt);
    getConfig().setString("MTSMSme.SCCP.hlr_gt", sccpHlrGt);

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

  public String getSccpUserId() {
    return String.valueOf(sccpUserId);
  }

  public void setSccpUserId(String sccpUserId) {
    try {
      this.sccpUserId = Integer.valueOf(sccpUserId).intValue();
    } catch (NumberFormatException e) {
      logger.error("Invalid MTSMSme.SCCP.userId parameter value: \"" + sccpUserId + '"', e);
      this.sccpUserId = 0;
    }
  }

  public String getSccpUserSsn() {
    return String.valueOf(sccpUserSsn);
  }

  public void setSccpUserSsn(String sccpUserSsn) {
    try {
      this.sccpUserSsn = Integer.valueOf(sccpUserSsn).intValue();
    } catch (NumberFormatException e) {
      logger.error("Invalid MTSMSme.SCCP.userSsn parameter value: \"" + sccpUserSsn + '"', e);
      this.sccpUserSsn = 0;
    }
  }

  public String getSccpMscGt() {
    return sccpMscGt;
  }

  public void setSccpMscGt(String sccpMscGt) {
    this.sccpMscGt = sccpMscGt;
  }

  public String getSccpVlrGt() {
    return sccpVlrGt;
  }

  public void setSccpVlrGt(String sccpVlrGt) {
    this.sccpVlrGt = sccpVlrGt;
  }

  public String getSccpHlrGt() {
    return sccpHlrGt;
  }

  public void setSccpHlrGt(String sccpHlrGt) {
    this.sccpHlrGt = sccpHlrGt;
  }

  public List getMappingSectionNames() {
    return new SortedList(getConfig().getSectionChildSectionNames(MAPPING_SECTION_NAME));
  }

  public int getInt(String paramName)
      throws Config.ParamNotFoundException, Config.WrongParamTypeException
  {
    return getConfig().getInt(paramName);
  }
  public String getString(String paramName)
      throws Config.WrongParamTypeException
  {
    try {
      return getConfig().getString(paramName);
    } catch (Config.ParamNotFoundException e) {
      return null;
    }
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

    for (Iterator i = requestParams.keySet().iterator(); i.hasNext();)
    {
      String paramName = (String) i.next();
      if (paramName.startsWith(PREFIX) && (paramName.endsWith("address") || paramName.endsWith("alias") || paramName.endsWith("mgt") || paramName.endsWith("msisdn") || paramName.endsWith("reg_type")
      || paramName.endsWith("comment")))
      {
        final String paramValue = getParamValue(requestParams.get(paramName));
        if (paramValue != null) getConfig().setString(paramName, paramValue);
      } else if (paramName.startsWith(PREFIX) && paramName.endsWith("period")) {
        final String paramValue = getParamValue(requestParams.get(paramName));
        try {
          if (paramValue != null) getConfig().setInt(paramName, Integer.parseInt(paramValue));
        } catch (NumberFormatException e) {
          logger.error(e, e);
          getConfig().setInt(paramName, 0);
        }
      }

    }

    if (mapping_new_address != null && mapping_new_address.length() > 0 &&
        mapping_new_reg_type != null && mapping_new_reg_type.length() > 0 &&
        mapping_new_alias != null && mapping_new_alias.length() > 0 &&
        mapping_new_mgt != null && mapping_new_mgt.length() > 0 &&
        mapping_new_msisdn != null && mapping_new_msisdn.length() > 0 &&
        mapping_new_period >= 0)
    {
      int i = mapping_new_address.lastIndexOf('.');
//      String section = mapping_new_address.replace('.', '_');
      String section = mapping_new_address.substring(i >=0 ? i : 0);
      getConfig().setString(PREFIX + section + ".address", mapping_new_address);
      getConfig().setString(PREFIX + section + ".comment", mapping_new_comment == null ? "" : mapping_new_comment);
      getConfig().setString(PREFIX + section + ".reg_type", mapping_new_reg_type);
      getConfig().setString(PREFIX + section + ".alias", mapping_new_alias);
      getConfig().setString(PREFIX + section + ".mgt", mapping_new_mgt);
      getConfig().setString(PREFIX + section + ".msisdn", mapping_new_msisdn);
      getConfig().setInt(PREFIX + section + ".period", mapping_new_period);
      System.out.println("Set period: "+PREFIX + section + ".period = "+mapping_new_period);
    }

    return RESULT_DONE;
  }

  public String getMapping_new_address() {
    return mapping_new_address;
  }
  public void setMapping_new_address(String mapping_new_address) {
    this.mapping_new_address = mapping_new_address;
  }
  public String getMapping_new_comment() {
    return mapping_new_comment;
  }

  public void setMapping_new_comment(String mapping_new_comment) {
    this.mapping_new_comment = mapping_new_comment;
  }

  public String getMapping_new_alias() {
    return mapping_new_alias;
  }
  public void setMapping_new_alias(String mapping_new_alias) {
    this.mapping_new_alias = mapping_new_alias;
  }
  public String getMapping_new_mgt() {
    return mapping_new_mgt;
  }
  public void setMapping_new_mgt(String mapping_new_mgt) {
    this.mapping_new_mgt = mapping_new_mgt;
  }
  public String getMapping_new_msisdn() {
    return mapping_new_msisdn;
  }
  public void setMapping_new_msisdn(String mapping_new_msisdn) {
    this.mapping_new_msisdn = mapping_new_msisdn;
  }
  public String getMapping_new_period() {
    return String.valueOf(mapping_new_period);
  }
  public void setMapping_new_period(String mapping_new_period) {
    try {
      this.mapping_new_period = Integer.valueOf(mapping_new_period).intValue();
    } catch (NumberFormatException e) {
      this.mapping_new_period = -1;
    }
  }

  public String getMapping_new_reg_type() {
    return mapping_new_reg_type;
  }

  public void setMapping_new_reg_type(String mapping_new_reg_type) {
    this.mapping_new_reg_type = mapping_new_reg_type;
  }

  protected abstract int apply();
  protected abstract int reset();
  protected abstract int start();
  protected abstract int stop();

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
