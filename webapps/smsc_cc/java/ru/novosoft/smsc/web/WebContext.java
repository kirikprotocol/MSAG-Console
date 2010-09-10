package ru.novosoft.smsc.web;

import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.xml.WebXml;
import ru.novosoft.smsc.web.auth.Authenticator;
import ru.novosoft.smsc.web.config.Configuration;
import ru.novosoft.smsc.web.config.SmscStatusManager;
import ru.novosoft.smsc.web.journal.Journal;

import java.util.concurrent.CountDownLatch;

/**
 * author: alkhal
 */
public class WebContext {

  private static WebContext instance;

  private WebXml webXml;

  private Authenticator authenticator;

  private AdminContext adminContext;


  private Configuration configuration;

  private SmscStatusManager smscStatusManager;

  private Journal journal;

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
    this.journal = new Journal();

    configuration = new Configuration(adminContext, journal);    

    smscStatusManager = new SmscStatusManager(adminContext);
  }

  public WebXml getWebXml() {
    return webXml;
  }

  public Authenticator getAuthenticator() {
    return authenticator;
  }

  public SmscStatusManager getSmscStatusManager() {
    return smscStatusManager;
  }

  public AdminContext getAdminContext() {
    return adminContext;
  }

  public Journal getJournal() {
    return journal;
  }

  public Configuration getConfiguration() {
    return configuration;
  }
}
