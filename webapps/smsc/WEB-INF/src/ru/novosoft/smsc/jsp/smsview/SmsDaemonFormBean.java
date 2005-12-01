package ru.novosoft.smsc.jsp.smsview;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.smsview.archive.ArchiveDaemonContext;
import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.util.config.Config;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 12.02.2004
 * Time: 12:42:02
 * To change this template use Options | File Templates.
 */
public class SmsDaemonFormBean extends PageBean
{
  public final static String LOC_SOURCES_SECTION = "ArchiveDaemon.Locations.sources";
  public final static String SME_PARAMS_SECTION = "ArchiveDaemon.Indexator.smeAddrChunkSize";

  private ArchiveDaemonContext daemonContext = null;

  private String mbStart = null;
  private String mbStop = null;
  private String mbSave = null;
  private String mbReset = null;
  private Map params = new HashMap();

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK) return result;

    try {
      daemonContext = ArchiveDaemonContext.getInstance(appContext);
    } catch (Exception exc) {
      exc.printStackTrace();
      return error(SMSCErrors.error.smsc.contextInitFailed, exc.getMessage());
    }

    return result;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK) return result;

    processParams(request);

    if (mbStart != null)
      return processStart();
    else if (mbStop != null)
      return processStop();
    else if (mbSave != null)
      return processApply();
    else if (mbReset != null) return processReset();

    return RESULT_OK;
  }

  private int processStart()
  {
    if (!getStatusOnline()) {
      try {
        hostsManager.startService(Constants.ARCHIVE_DAEMON_SVC_ID);
        return RESULT_OK;
      } catch (Throwable e) {
        logger.error("Couldn't start Archive Daemon", e);
        return error(SMSCErrors.error.smsc.couldntStart, e);
      }
    }
    else
      return RESULT_OK;
  }

  private int processStop()
  {
    if (getStatusOnline()) {
      try {
        hostsManager.shutdownService(Constants.ARCHIVE_DAEMON_SVC_ID);
        return RESULT_OK;
      } catch (Throwable e) {
        logger.error("Couldn't stop Archive Daemon", e);
        return error(SMSCErrors.error.smsc.couldntStop, e);
      }
    }
    else
      return RESULT_OK;
  }

  public boolean getStatusOnline()
  {
    try {
      return hostsManager.getServiceInfo(Constants.ARCHIVE_DAEMON_SVC_ID).isOnline();
    } catch (AdminException e) {
      return false;
    }
  }

  private int processApply()
  {
    Config config = daemonContext.getConfig();
    if (config == null)
      return error(SMSCErrors.error.smsc.couldntGetConfig);

    config.removeParamsFromSection(LOC_SOURCES_SECTION);
    config.removeParamsFromSection(SME_PARAMS_SECTION);

    for (Iterator i = params.keySet().iterator(); i.hasNext();) {
      String name = (String) i.next();
      Object value = params.get(name);
      if (value instanceof Boolean)
        config.setBool(name, ((Boolean) value).booleanValue());
      else if (value instanceof Integer)
        config.setInt(name, ((Integer) value).intValue());
      else
        config.setString(name, (String) value);
    }
    try {
      daemonContext.saveConfig(config);
    } catch (Exception e) {
      logger.error("Couldn't save new Archive Daemon config", e);
      return error(SMSCErrors.error.smsc.couldntSave);
    }
    return RESULT_OK;
  }

  private int processReset()
  {
    params.clear();
    return loadParams();
  }

  private int loadParams()
  {
    Config config = daemonContext.getConfig();
    if (config == null)
      return error(SMSCErrors.error.smsc.couldntGetConfig);

    for (Iterator i = config.getParameterNames().iterator(); i.hasNext();) {
      String name = (String) i.next();
      params.put(name, config.getParameter(name));
    }
    return RESULT_OK;
  }

  private void clearChangeableParams()
  {
    ArrayList toRemove = new ArrayList();
    for (Iterator i = params.keySet().iterator(); i.hasNext();) {
      String paramName = (String) i.next();
      if (paramName.startsWith(LOC_SOURCES_SECTION) || paramName.startsWith(SME_PARAMS_SECTION)) {
        toRemove.add(paramName);
      }
    }
    for (int i = 0; i < toRemove.size(); i++) params.remove(toRemove.get(i));
  }

  private int processParams(HttpServletRequest request)
  {
    int result = RESULT_OK;
    if (params.size() == 0) loadParams();

    boolean isRequestHaveParams = false;
    boolean needCleanChangeable = true;

    Enumeration parameterNames = request.getParameterNames();

    while (parameterNames.hasMoreElements()) {
      String s = (String) parameterNames.nextElement();
      if (s.indexOf('.') <= 0 || s.startsWith("newParamName_") || s.startsWith("newParamValue_")) continue;

      if (needCleanChangeable) {
        clearChangeableParams();
        needCleanChangeable = false;
      }
      isRequestHaveParams = true;
      Object oldValue = params.get(s);
      final String parameter = request.getParameter(s);
      //System.out.println("Param: "+s+"="+parameter+", old: "+oldValue);
      if (oldValue != null) {
        if (oldValue instanceof Integer) {
          try {
            if (parameter != null && parameter.trim().length() > 0)
              params.put(s, Integer.decode(parameter.trim()));
            else
              params.put(s, new Integer(0));
          } catch (NumberFormatException e) {
            logger.error("Invalid integer parameter: " + s + "=" + parameter);
            result = error(SMSCErrors.error.smsc.invalidIntParameter, s);
          }
        }
        else if (oldValue instanceof Boolean)
          params.put(s, Boolean.valueOf(parameter));
        else
          params.put(s, parameter);
      }
      else {
        if (s.startsWith(LOC_SOURCES_SECTION)) {
          params.put(s, parameter);
        }
        else if (s.startsWith(SME_PARAMS_SECTION)) {
          try {
            if (parameter != null && parameter.trim().length() > 0)
              params.put(s, Integer.decode(parameter.trim()));
            else
              params.put(s, new Integer(0));
          } catch (NumberFormatException e) {
            logger.error("Invalid integer parameter: " + s + "=" + parameter);
            result = error(SMSCErrors.error.smsc.invalidIntParameter, s);
          }
        }
        else {
          try {
            params.put(s, Integer.decode(parameter));
          } catch (NumberFormatException e) {
            if (parameter.equalsIgnoreCase("true"))
              params.put(s, Boolean.TRUE);
            else if (parameter.equalsIgnoreCase("false"))
              params.put(s, Boolean.FALSE);
            else
              params.put(s, parameter);
          }
        }
      }
    }

    if (isRequestHaveParams) {
      for (Iterator i = params.keySet().iterator(); i.hasNext();) {
        String paramName = (String) i.next();
        Object value = params.get(paramName);
        if (value instanceof Boolean) {
          final String parameter = request.getParameter(paramName);
          params.put(paramName, Boolean.valueOf(parameter));
        }
      }
    }
    return result;
  }

  public HashMap getSubParams(String sectionName)
  {
    HashMap subParams = new HashMap();
    for (Iterator i = params.keySet().iterator(); i.hasNext();) {
      String paramName = (String) i.next();
      if (!paramName.startsWith(sectionName)) continue;
      String shortName = paramName.substring(sectionName.length() + 1);
      if (shortName.indexOf('.') != -1) continue;
      subParams.put(shortName, params.get(paramName));
    }
    return subParams;
  }

  public String getStringParam(String paramName)
  {
    Object param = params.get(paramName);
    if (param == null) return "<" + appContext.getLocaleString("common.util.notSpecified") + ">";
    if (param instanceof String) return (String) param;
    return null;
  }

  public int getIntParam(String paramName)
  {
    Object param = params.get(paramName);
    if (param == null) {
      logger.error("integer parameter \"" + paramName + "\" not found.");
      return 0;
    }
    if (param instanceof Integer) {
      return ((Integer) param).intValue();
    }
    else {
      logger.error("parameter \"" + paramName + "\" is not integer.");
      return -1;
    }
  }

  public boolean getBoolParam(String paramName)
  {
    Object param = params.get(paramName);
    if (param == null) {
      logger.error("boolean parameter \"" + paramName + "\" not found.");
      return false;
    }
    if (param instanceof Boolean) {
      return ((Boolean) param).booleanValue();
    }
    else {
      logger.error("parameter \"" + paramName + "\" is not boolean.");
      return false;
    }
  }

  public void setStringParam(String paramName, String paramValue)
  {
    params.put(paramName, paramValue);
  }

  public void setIntParam(String paramName, int paramValue)
  {
    params.put(paramName, new Integer(paramValue));
  }

  public void setBoolParam(String paramName, boolean paramValue)
  {
    params.put(paramName, new Boolean(paramValue));
  }

  public String getMbStart()
  {
    return mbStart;
  }

  public void setMbStart(String mbStart)
  {
    this.mbStart = mbStart;
  }

  public String getMbStop()
  {
    return mbStop;
  }

  public void setMbStop(String mbStop)
  {
    this.mbStop = mbStop;
  }

  public String getMbSave()
  {
    return mbSave;
  }

  public void setMbSave(String mbSave)
  {
    this.mbSave = mbSave;
  }

  public String getMbReset()
  {
    return mbReset;
  }

  public void setMbReset(String mbReset)
  {
    this.mbReset = mbReset;
  }
}
