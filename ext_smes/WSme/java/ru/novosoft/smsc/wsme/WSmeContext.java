/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: May 13, 2003
 * Time: 6:07:38 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.wsme;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.service.Service;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.jsp.SMSCAppContext;

import java.security.Principal;
import java.util.HashMap;
import java.util.Map;

public class WSmeContext
{
  private static Map instances = new HashMap();

  private WSme wsme = null;
  private SMSCAppContext appContext;
  private HashMap preferences = new HashMap();

  private WSmeContext(SMSCAppContext appContext, String smeId)
  {
    this.appContext = appContext;
    try {
      ServiceInfo wsmeServiceInfo = this.appContext.getHostsManager().getServiceInfo(smeId);
      wsme = new WSme(wsmeServiceInfo);

      wsme.init(wsmeServiceInfo);
    } catch (AdminException e) {
      System.out.println("Exception in initialization:");
      e.printStackTrace();
    }
  }

  public static WSmeContext getInstance(SMSCAppContext appContext, String smeId)
  {
    WSmeContext instance = (WSmeContext) instances.get(smeId);
    if (instance == null) {
      instance = new WSmeContext(appContext, smeId);
      instances.put(smeId, instance);
    }
    return instance;
  }

  synchronized public WSmePreferences getWSmePreferences(Principal loginedUser)
  {
    WSmePreferences userPreferences = null;
    Object obj = preferences.get(loginedUser.getName());
    if (obj != null && obj instanceof WSmePreferences) {
      System.out.println("Got old prefs for: " + loginedUser.getName());
      userPreferences = (WSmePreferences) obj;
    } else {
      userPreferences = new WSmePreferences();
      System.out.println("Got new prefs for: " + loginedUser.getName());
      preferences.put(loginedUser.getName(), userPreferences);
    }
    return userPreferences;
  }

  synchronized public SMSCAppContext getAppContext()
  {
    return appContext;
  }

  synchronized public WSme getWsme()
  {
    return wsme;
  }
}
