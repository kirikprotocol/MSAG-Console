package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.infosme.backend.config.driver.Driver;
import ru.novosoft.smsc.util.SortedList;

import javax.servlet.http.HttpServletRequest;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

/**
 * Created by igork
 * Date: Jul 31, 2003
 * Time: 3:43:49 PM
 */
public class Drivers extends InfoSmeBean
{

  private String dsdriver_new_type = null;
  private String dsdriver_new_loadup = null;

  private String mbDone = null;
  private String mbCancel = null;

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (mbCancel != null)
      return RESULT_DONE;
    else if (mbDone != null)
      return done(request.getParameterMap());

    return result;
  }

  private int done(Map requestParams)
  {
    getInfoSmeConfig().clearDrivers();

    for (Iterator i = requestParams.keySet().iterator(); i.hasNext();) {
      String paramName = (String) i.next();
      if (paramName.indexOf(".loadup") > 0) {
        String driverName = paramName.substring(0, paramName.indexOf(".loadup"));
        Driver d = getInfoSmeConfig().getDriver(driverName);
        if (d == null) {
          d = new Driver();
          d.setType(driverName);
          d.setLoadup(getParamValue(requestParams.get(paramName)));
          getInfoSmeConfig().addDriver(d);
        }
      }
    }

    if (dsdriver_new_type != null && dsdriver_new_type.length() > 0
            && dsdriver_new_loadup != null && dsdriver_new_loadup.length() > 0) {
      Driver d = new Driver();
      d.setType(dsdriver_new_type);
      d.setLoadup(dsdriver_new_loadup);
      getInfoSmeConfig().addDriver(d);
    }
    return RESULT_DONE;
  }

  private static String getParamValue(Object paramObjectValue)
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

  public List getDriverTypes()
  {
    List drivers = getInfoSmeConfig().getDrivers();
    List names = new ArrayList(drivers.size());
    for (Iterator iter = drivers.iterator(); iter.hasNext();)
      names.add(((Driver)iter.next()).getType());    
    return new SortedList(names);
  }

  public String getDriverLoadup(String driver) {
    return getInfoSmeConfig().getDriver(driver).getLoadup();
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
