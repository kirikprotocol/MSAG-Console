/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: May 13, 2003
 * Time: 6:07:38 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.wsme;

import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.admin.service.Service;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.AdminException;

import java.security.Principal;
import java.util.HashMap;

public class WSmeContext
{
  private static WSmeContext instance = null;

  private WSme wsme = null;
  private Service wsmeService = null;
  private SMSCAppContext appContext;
  private HashMap preferences = new HashMap();

  private WSmeContext(SMSCAppContext appContext)
  {
    this.appContext = appContext;
    try {
      wsmeService = this.appContext.getHostsManager().getService(Constants.WSME_SME_ID);
      wsme = new WSme(wsmeService);      wsme.init(wsmeService);
    } catch (AdminException e) {
      System.out.println("Exception in initialization:");
      e.printStackTrace();
    }
  }
  public static void init(SMSCAppContext appContext) {
    if (instance == null) {
      instance = new WSmeContext(appContext);
    }
  }
  public static WSmeContext getInstance() {
    return instance;
  }

  synchronized public WSmePreferences getWSmePreferences(Principal loginedUser)
  {
    WSmePreferences userPreferences = null;
    Object obj = preferences.get(loginedUser.getName());
    if (obj != null && obj instanceof WSmePreferences) {
      System.out.println("Got old prefs for: "+loginedUser.getName());
      userPreferences = (WSmePreferences)obj;
    } else {
      userPreferences = new WSmePreferences();
      System.out.println("Got new prefs for: "+loginedUser.getName());
      preferences.put(loginedUser.getName(), userPreferences);
    }
    return userPreferences;
  }
  synchronized public SMSCAppContext getAppContext() {
    return appContext;
  }
  synchronized public Service getWsmeService() {
    return wsmeService;
  }
  synchronized public WSme getWsme() {
    return wsme;
  }
}
