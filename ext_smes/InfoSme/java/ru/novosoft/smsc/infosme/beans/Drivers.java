package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.SortedList;
import ru.novosoft.smsc.util.config.Config;

import java.security.Principal;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

/**
 * Created by igork
 * Date: Jul 31, 2003
 * Time: 3:43:49 PM
 */
public class Drivers extends InfoSmeBean {
  public static final String DRIVERS_SECTION_NAME = "StartupLoader.DataSourceDrivers";

  private String dsdriver_new_type = null;
  private String dsdriver_new_loadup = null;

  private String mbDone = null;
  private String mbCancel = null;

  public int process(SMSCAppContext appContext, List errors, Principal loginedPrincipal, Map requestParams)
  {
    int result = super.process(appContext, errors, loginedPrincipal);
    if (result != RESULT_OK)
      return result;

    if (mbCancel != null)
      return RESULT_DONE;
    else if (mbDone != null)
      return done(requestParams);

    return result;
  }

  private int done(Map requestParams)
  {
    final String PREFIX = DRIVERS_SECTION_NAME + '.';
    getConfig().removeSection(DRIVERS_SECTION_NAME);
    for (Iterator i = requestParams.keySet().iterator(); i.hasNext();) {
      String paramName = (String) i.next();
      if (paramName.startsWith(PREFIX)) {
        final String paramValue = getParamValue(requestParams.get(paramName));
        if (paramValue != null)
          getConfig().setString(paramName, paramValue);
      }
    }
    if (dsdriver_new_type != null && dsdriver_new_type.length() > 0
        && dsdriver_new_loadup != null && dsdriver_new_loadup.length() > 0) {
      getConfig().setString(PREFIX + dsdriver_new_type + ".type", dsdriver_new_type);
      getConfig().setString(PREFIX + dsdriver_new_type + ".loadup", dsdriver_new_loadup);
    }
    return RESULT_DONE;
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
    } else
      return null;
  }

  public List getDriverSectionNames()
  {
    return new SortedList(getConfig().getSectionChildSectionNames(DRIVERS_SECTION_NAME));
  }

  public int getInt(String paramName) throws Config.ParamNotFoundException, Config.WrongParamTypeException
  {
    return getConfig().getInt(paramName);
  }

  public String getString(String paramName) throws Config.ParamNotFoundException, Config.WrongParamTypeException
  {
    return getConfig().getString(paramName);
  }

  public boolean getBool(String paramName) throws Config.ParamNotFoundException, Config.WrongParamTypeException
  {
    return getConfig().getBool(paramName);
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

  public String getDsdriver_new_type()
  {
    return dsdriver_new_type;
  }

  public void setDsdriver_new_type(String dsdriver_new_type)
  {
    this.dsdriver_new_type = dsdriver_new_type;
  }

  public String getDsdriver_new_loadup()
  {
    return dsdriver_new_loadup;
  }

  public void setDsdriver_new_loadup(String dsdriver_new_loadup)
  {
    this.dsdriver_new_loadup = dsdriver_new_loadup;
  }
}
