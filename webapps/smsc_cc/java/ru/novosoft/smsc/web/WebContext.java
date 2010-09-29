package ru.novosoft.smsc.web;

import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.acl.AclManager;
import ru.novosoft.smsc.admin.alias.AliasManager;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonManager;
import ru.novosoft.smsc.util.xml.WebXml;
import ru.novosoft.smsc.web.auth.Authenticator;
import ru.novosoft.smsc.web.config.Configuration;
import ru.novosoft.smsc.web.config.SmscStatusManager;
import ru.novosoft.smsc.web.config.acl.WAclManager;
import ru.novosoft.smsc.web.config.alias.WAliasManager;
import ru.novosoft.smsc.web.config.archive_daemon.WArchiveDaemonManager;
import ru.novosoft.smsc.web.journal.Journal;

import javax.faces.context.FacesContext;
import javax.servlet.http.HttpSession;
import java.security.Principal;
import java.util.concurrent.CountDownLatch;

/**
 * author: alkhal
 */
public class WebContext {

  private static final CountDownLatch initLatch = new CountDownLatch(1);

  private static Authenticator auth;
  private static WebXml webXml;
  private static AdminContext adminContext;
  private static Journal journal = new Journal();
  private static WebContext nullUserInstance;

  private final Configuration configuration;
  private final SmscStatusManager smscStatusManager;
  private final AclManager aclManager;
  private final AliasManager aliasManager;
  private final WArchiveDaemonManager archiveDaemonManager;

  public static void init(Authenticator authenticator, WebXml webXml, AdminContext adminContext) throws InitException {
    auth = authenticator;
    WebContext.webXml = webXml;
    WebContext.adminContext = adminContext;
    nullUserInstance = new WebContext(null);
    initLatch.countDown();
  }

  public static WebContext getInstance() {
    try {
      initLatch.await();
      Principal user = null;
      if (FacesContext.getCurrentInstance() != null)
        user = FacesContext.getCurrentInstance().getExternalContext().getUserPrincipal();
      if (user == null)
        return nullUserInstance;

      HttpSession s = (HttpSession) FacesContext.getCurrentInstance().getExternalContext().getSession(true);
      WebContext ctx = (WebContext) s.getAttribute(WebContext.class.getCanonicalName());
      if (ctx == null) {
        ctx = new WebContext(user.getName());
        s.setAttribute(WebContext.class.getCanonicalName(), ctx);
      }

      return ctx;
    } catch (InterruptedException e) {
      return null;
    }
  }

  public WebContext(String user) {
    configuration = new Configuration(adminContext, journal);
    smscStatusManager = new SmscStatusManager(adminContext);

    aclManager = new WAclManager(adminContext.getAclManager(), journal, user);
    aliasManager = new WAliasManager(adminContext.getAliasManager(), user, journal);
    archiveDaemonManager = new WArchiveDaemonManager(adminContext.getArchiveDaemonManager(), journal, user);
  }

  public WebXml getWebXml() {
    return webXml;
  }

  public static Authenticator getAuthenticator() {
    try {
      initLatch.await();
      return auth;
    } catch (InterruptedException e) {
      return null;
    }
  }

  public SmscStatusManager getSmscStatusManager() {
    return smscStatusManager;
  }

  public AclManager getAclManager() {
    return aclManager;
  }

  public AliasManager getAliasManager() {
    return aliasManager;
  }

  public WArchiveDaemonManager getArchiveDaemonManager() {
    return archiveDaemonManager;
  }

  public Journal getJournal() {
    return journal;
  }

  public Configuration getConfiguration() {
    return configuration;
  }
}
