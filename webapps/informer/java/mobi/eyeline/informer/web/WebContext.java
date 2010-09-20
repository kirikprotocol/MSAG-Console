package mobi.eyeline.informer.web;

import mobi.eyeline.informer.admin.AdminContext;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.users.UsersSettings;
import mobi.eyeline.informer.util.xml.WebXml;
import mobi.eyeline.informer.web.auth.Authenticator;

import java.util.concurrent.CountDownLatch;

/**
 * author: alkhal
 */
public class WebContext {

  private static WebContext instance;

  private WebXml webXml;

  private Authenticator authenticator;

  private AdminContext adminContext;

  private static final CountDownLatch initLatch = new CountDownLatch(1);

  public static void init(Authenticator authenticator, WebXml webXml, AdminContext adminContext) throws InitException {
    if (instance == null) {
      instance = new WebContext(authenticator, webXml, adminContext);
      initLatch.countDown();
    }
  }

  public static WebContext getInstance() {
    try {
      initLatch.await();
      return instance;
    } catch (InterruptedException e) {
      return null;
    }
  }

  public WebContext(Authenticator authenticator, WebXml webXml, AdminContext adminContext) throws InitException {
    this.authenticator = authenticator;
    this.webXml = webXml;
    this.adminContext = adminContext;
  }

  public WebXml getWebXml() {
    return webXml;
  }

  public Authenticator getAuthenticator() {
    return authenticator;
  }


  public UsersSettings getUserSettings() throws AdminException{
    return adminContext.getUsersManager().getUsersSettings();
  }

}
