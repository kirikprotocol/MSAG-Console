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

public class WSmeContext
{
  private static WSmeContext instance = null;

  private WSme wsme = null;
  private Service wsmeService = null;
  private SMSCAppContext appContext;
  private Principal loginedUserPrincipal;

  private WSmeContext(SMSCAppContext appContext, Principal loginedUserPrincipal)
  {
    this.appContext = appContext;
    this.loginedUserPrincipal = loginedUserPrincipal;
    try {
      wsmeService = this.appContext.getHostsManager().getService(Constants.WSME_SME_ID);
      wsme = new WSme(wsmeService);
    } catch (AdminException e) {
      System.out.println("Exception in initialization:");
      e.printStackTrace();
    }
  }
  public static void init(SMSCAppContext appContext, Principal loginedUserPrincipal) {
    if (instance == null) {
      instance = new WSmeContext(appContext, loginedUserPrincipal);
    }
  }
  public static WSmeContext getInstance() {
    return instance;
  }

  synchronized public Service getWsmeService() {
    return wsmeService;
  }
  synchronized public WSme getWsme() {
    return wsme;
  }
  synchronized public Principal getLoginedUserPrincipal() {
    return loginedUserPrincipal;
  }
  synchronized public SMSCAppContext getAppContext() {
    return appContext;
  }
}
