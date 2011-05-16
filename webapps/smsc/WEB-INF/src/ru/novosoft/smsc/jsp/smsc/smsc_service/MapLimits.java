package ru.novosoft.smsc.jsp.smsc.smsc_service;

import ru.novosoft.smsc.jsp.smsc.SmscBean;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.AdminException;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

/**
 * User: artem
 * Date: 20.03.2008
 */

public class MapLimits extends SmscBean {

  private String mbSave = null;
  private String mbReset = null;
  private Map params = new HashMap();

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    result = processParams(appContext, request) ;
    if (result != RESULT_OK) {
      return result;
    }

    if (mbSave != null)
      return processApply(appContext);
    else if (mbReset != null)
      return processReset(appContext);
    else
      return RESULT_OK;
  }

  private int processApply(SMSCAppContext appContext)
  {
    Config config = appContext.getSmsc().getMapLimitsConfig();
    if (config == null)
      return error(SMSCErrors.error.smsc.couldntGetConfig);

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
      appContext.getSmsc().saveMapLimitsConfig(config);
      journalAppend(SubjectTypes.TYPE_maplimits, null, Actions.ACTION_MODIFY);
    } catch (AdminException e) {
      logger.error("Couldn't save MAP limits config", e);
      return error(SMSCErrors.error.smsc.couldntSave, e.getMessage());
    }

    try {
      appContext.getSmsc().applyMapLimitsConfig();
    } catch (AdminException e) {
      logger.error("Couldn't apply MAP limits config", e);
      return error(SMSCErrors.error.smsc.couldntApply, e.getMessage());
    }
    return RESULT_OK;
  }

  private int processReset(SMSCAppContext appContext)
  {
    params.clear();
    return loadParams(appContext);
  }

  private int loadParams(SMSCAppContext appContext)
  {
    Config config = appContext.getSmsc().getMapLimitsConfig();
    if (config == null)
      return error(SMSCErrors.error.smsc.couldntGetConfig);

    for (Iterator i = config.getParameterNames().iterator(); i.hasNext();) {
      String name = (String) i.next();
      params.put(name, config.getParameter(name));
    }

    return RESULT_OK;
  }

  private int processParams(SMSCAppContext appContext, HttpServletRequest request)
  {
    int result = RESULT_OK;
    if (params.isEmpty())
      loadParams(appContext);

    boolean isRequestHaveParams = false;

    Enumeration parameterNames = request.getParameterNames();
    while (parameterNames.hasMoreElements()) {
      String s = (String) parameterNames.nextElement();
      if (s.indexOf('.') > 0) {
        isRequestHaveParams = true;
        Object oldValue = params.get(s);
        final String parameter = request.getParameter(s);
        if (oldValue != null) {
          if (oldValue instanceof Integer) {
            try {
              if (parameter != null && parameter.trim().length() > 0) {
                Integer value = Integer.decode(parameter.trim());
                if (value.intValue() > 0)
                  params.put(s, Integer.decode(parameter.trim()));
                else
                  throw new NumberFormatException();
              } else
                  throw new NumberFormatException();
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
          if (parameter != null)
            params.put(paramName, Boolean.valueOf(parameter));
        }
      }
    }
    return result;
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

  public void setIntParam(String paramName, int paramValue)
  {
    params.put(paramName, new Integer(paramValue));
  }

  public String getStringParam(String paramName)
  {
    Object param = params.get(paramName);
    if (param == null)
      return "";
    if (param instanceof String)
      return (String) param;
    else
      return null;
  }

  public void setStringParam(String paramName, String paramValue)
  {
    params.put(paramName, paramValue);
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
