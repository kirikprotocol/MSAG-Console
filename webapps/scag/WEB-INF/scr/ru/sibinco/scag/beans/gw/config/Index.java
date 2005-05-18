package ru.sibinco.scag.beans.gw.config;

import ru.sibinco.lib.backend.util.config.Config;
import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.Statuses;
import ru.sibinco.scag.beans.*;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.util.*;


/**
 * Created by IntelliJ IDEA. User: igork Date: 03.03.2004 Time: 18:39:37
 */
public class Index extends EditBean
{
  private Map params = new HashMap();
  private Map requestParams = null;
  private static final String COLLAPSING_TREE_PARAM_PREFIX = "collapsing_tree_param.";

  public void process(HttpServletRequest request, HttpServletResponse response) throws SCAGJspException
  {
    requestParams = request.getParameterMap();
    super.process(request, response);
  }

  public String getId()
  {
    return null;
  }

  protected void load(String loadId) throws SCAGJspException
  {
    for (Iterator i = requestParams.entrySet().iterator(); i.hasNext();) {
      Map.Entry entry = (Map.Entry) i.next();
      if (entry.getKey() instanceof String) {
        String key = (String) entry.getKey();
        if (key.startsWith(COLLAPSING_TREE_PARAM_PREFIX)) {
          String name = key.substring(COLLAPSING_TREE_PARAM_PREFIX.length());
          StringBuffer value = new StringBuffer();
          for (int j = 0; j < ((String[]) entry.getValue()).length; j++) {
            String valueElem = ((String[]) entry.getValue())[j];
            value.append(valueElem);
          }
          params.put(name, value.toString());
        }
      }
    }
    if (params.size() == 0) {
      final Config gwConfig = appContext.getGwConfig();
      for (Iterator i = gwConfig.getParameterNames().iterator(); i.hasNext();) {
        String name = (String) i.next();
        Object value = gwConfig.getParameter(name);
        if (value instanceof String)
          params.put(name, value);
        else if (value instanceof Boolean) {
          Boolean valueBoolean = (Boolean) value;
          params.put(name, String.valueOf(valueBoolean.booleanValue()));
        } else if (value instanceof Long) {
          Long aLong = (Long) value;
          params.put(name, String.valueOf(aLong.longValue()));
        } else if (value instanceof Integer) {
          Integer integer = (Integer) value;
          params.put(name, String.valueOf(integer.longValue()));
        }
      }
    }
  }

  protected void save() throws SCAGJspException
  {
    final Config gwConfig = appContext.getGwConfig();
    final Statuses statuses = appContext.getStatuses();
    for (Iterator i = params.entrySet().iterator(); i.hasNext();) {
      Map.Entry entry = (Map.Entry) i.next();
      final Object parameter = gwConfig.getParameter((String) entry.getKey());
      if (parameter != null) {
        if (parameter instanceof String)
          gwConfig.setString((String) entry.getKey(), (String) entry.getValue());
        else if (parameter instanceof Integer || parameter instanceof Long)
          try {
            gwConfig.setInt((String) entry.getKey(), Integer.parseInt((String) entry.getValue()));
          } catch (NumberFormatException e) {
            throw new SCAGJspException(Constants.errors.config.INVALID_INTEGER, (String) entry.getValue());
          }
        else if (parameter instanceof Boolean)
          gwConfig.setBool((String) entry.getKey(), Boolean.valueOf((String) entry.getValue()).booleanValue());
        else
          gwConfig.setString((String) entry.getKey(), (String) entry.getValue());
      } else
        gwConfig.setString((String) entry.getKey(), (String) entry.getValue());
      statuses.setConfigChanged(true);
    }
    throw new DoneException();
  }

  public Map getParams()
  {
    return params;
  }

  public void setParams(Map params)
  {
    this.params = params;
  }
}
