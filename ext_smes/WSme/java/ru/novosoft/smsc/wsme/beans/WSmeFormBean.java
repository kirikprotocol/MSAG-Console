/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: May 13, 2003
 * Time: 5:18:40 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.wsme.beans;

import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.wsme.WSmeContext;
import ru.novosoft.smsc.wsme.WSme;
import ru.novosoft.smsc.wsme.WSmeErrors;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.service.ServiceInfo;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

public class WSmeFormBean extends IndexBean
{
  public final static int RESULT_VISITORS = 1000;
  public final static int RESULT_HISTORY = 2000;
  public final static int RESULT_LANGS = 3000;
  public final static int RESULT_ADS = 4000;

  protected WSme wsme = null;

  private Config config = null;
  private Map params = new HashMap();
  private String btnApply  = null;
  private String btnCancel = null;

  private int menuSelection = RESULT_OK;

  public int process(HttpServletRequest request, List errors)
  {
    int result = this.process(errors);
    if (result == RESULT_OK || result == RESULT_DONE)
    {
      result = processParams(request);
      if (result == RESULT_OK)
      {
        if (btnApply != null)
          result = processApply();
        else if (btnCancel != null)
          result = processReset();
      }
    }

    btnApply = null; btnCancel = null;
    return result;
  }

  protected int process(List errors)
  {
    WSmeContext wSmeContext = WSmeContext.getInstance();
    if (wsme == null) wsme = wSmeContext.getWsme();

    int result = super.process(wSmeContext.getAppContext(),
                               errors, wSmeContext.getLoginedUserPrincipal());
    if (result != RESULT_OK)
      return result;

    if (wsme == null)
      return error(WSmeErrors.error.admin.InitFailed, "WSme initialized incorrectly");

    if (menuSelection != RESULT_OK)
      return menuSelection; // redirect

    return RESULT_OK;
  }

  private int processApply()
  {
    if (config == null)
      return error(WSmeErrors.error.admin.ConfigError, "Couldn't access config");

    for (Iterator i = params.keySet().iterator(); i.hasNext();)
    {
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
      wsme.reload(config);
    } catch (Exception e) {
      return error(WSmeErrors.error.admin.ApplyFailed, e.getMessage());
    }

    return RESULT_OK;
  }

  private int processReset()
  {
    params.clear();
    return loadParams();
  }

  public byte getWSmeStatus()
  {
    try {
      return hostsManager.getServiceInfo(Constants.WSME_SME_ID).getStatus();
    }
    catch (AdminException e) {
      return ServiceInfo.STATUS_UNKNOWN;
    }
  }
  public boolean isWSmeStarted() {
    return (getWSmeStatus() == ServiceInfo.STATUS_RUNNING);
  }

  private int loadParams()
  {
    try {
      if (config == null) config = wsme.getConfig();
    } catch (Exception e) {
      return error(WSmeErrors.error.admin.ConfigError, e.getMessage());
    }

    for (Iterator i = config.getParameterNames().iterator(); i.hasNext();)
    {
      String name = (String) i.next();
      params.put(name, config.getParameter(name));
    }
    return RESULT_OK;
  }

  private int processParams(HttpServletRequest request)
  {
    int result = RESULT_OK;
    if (params.size() == 0)
      loadParams();

    boolean isRequestHaveParams = false;

    Enumeration parameterNames = request.getParameterNames();
    while (parameterNames.hasMoreElements())
    {
      String s = (String) parameterNames.nextElement();
      if (s.indexOf('.') > 0)
      {
        isRequestHaveParams = true;
        Object oldValue = params.get(s);
        if (oldValue != null)
        {
          if (oldValue instanceof Integer)
          {
            String parameter = request.getParameter(s);
            try
            {
              if (parameter != null && parameter.trim().length() > 0)
                params.put(s, Integer.decode(parameter.trim()));
              else
                params.put(s, new Integer(0));
            }
            catch (NumberFormatException e)
            {
              logger.error("Invalid integer parameter: " + s + "=" + parameter);
              result = error(WSmeErrors.error.admin.InvalidIntParam, s);
            }
          }
          else if (oldValue instanceof Boolean)
            params.put(s, Boolean.valueOf(request.getParameter(s)));
          else
            params.put(s, request.getParameter(s));
        }
        else
          params.put(s, request.getParameter(s));
      }
    }

    if (isRequestHaveParams)
    {
      for (Iterator i = params.keySet().iterator(); i.hasNext();)
      {
        String paramName = (String) i.next();
        Object value = params.get(paramName);
        if (value instanceof Boolean)
        {
          final String parameter = request.getParameter(paramName);
          params.put(paramName, Boolean.valueOf(parameter));
        }
      }
    }
    return result;
  }

  public String getStringParam(String paramName)
  {
    Object param = params.get(paramName);
    if (param == null)
      return "not found " + params.size();
    if (param instanceof String)
      return (String) param;
    else
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
    else  {
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

  public Set getDatasourceDrivers()
  {
    Set result = new HashSet();
    final String prefix = "StartupLoader.DataSourceDrivers.";
    for (Iterator i = params.keySet().iterator(); i.hasNext();)
    {
      String name = (String) i.next();
      if (name.startsWith(prefix))
      {
        String s = name.substring(prefix.length());
        result.add(s.substring(0, s.indexOf('.')));
      }
    }
    return result;
  }

  public void setStringParam(String paramName, String paramValue) {
    params.put(paramName, paramValue);
  }
  public void setIntParam(String paramName, int paramValue) {
    params.put(paramName, new Integer(paramValue));
  }
  public void setBoolParam(String paramName, boolean paramValue) {
    params.put(paramName, new Boolean(paramValue));
  }

  public int getMenuSelection() {
    return menuSelection;
  }
  public void setMenuSelection(int menuSelection) {
    this.menuSelection = menuSelection;
  }
  public String getBtnApply() {
    return btnApply;
  }
  public void setBtnApply(String btnApply) {
    this.btnApply = btnApply;
  }
  public String getBtnCancel() {
    return btnCancel;
  }
  public void setBtnCancel(String btnCancel) {
    this.btnCancel = btnCancel;
  }
  public int getMenuId() {
    return RESULT_DONE;
  }
}
