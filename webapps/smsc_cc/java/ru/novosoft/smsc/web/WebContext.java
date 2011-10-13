package ru.novosoft.smsc.web;

import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.acl.AclManager;
import ru.novosoft.smsc.admin.alias.AliasManager;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemon;
import ru.novosoft.smsc.admin.closed_groups.ClosedGroupManager;
import ru.novosoft.smsc.admin.msc.MscManager;
import ru.novosoft.smsc.admin.operative_store.OperativeStoreManager;
import ru.novosoft.smsc.admin.profile.ProfileManager;
import ru.novosoft.smsc.admin.resource.ResourceManager;
import ru.novosoft.smsc.admin.sme.SmeManager;
import ru.novosoft.smsc.admin.stat.SmscStatProvider;
import ru.novosoft.smsc.util.xml.WebXml;
import ru.novosoft.smsc.web.auth.Authenticator;
import ru.novosoft.smsc.web.config.acl.WAclManager;
import ru.novosoft.smsc.web.config.alias.WAliasManager;
import ru.novosoft.smsc.web.config.archive_daemon.WArchiveDaemonManager;
import ru.novosoft.smsc.web.config.category.WCategoryManager;
import ru.novosoft.smsc.web.config.closed_groups.WClosedGroupManager;
import ru.novosoft.smsc.web.config.fraud.WFraudManager;
import ru.novosoft.smsc.web.config.logging.WLoggerManager;
import ru.novosoft.smsc.web.config.map_limit.WMapLimitManager;
import ru.novosoft.smsc.web.config.msc.WMscManager;
import ru.novosoft.smsc.web.config.profile.WProfileManager;
import ru.novosoft.smsc.web.config.provider.WProviderManager;
import ru.novosoft.smsc.web.config.reschedule.WRescheduleManager;
import ru.novosoft.smsc.web.config.resource.WResourceManager;
import ru.novosoft.smsc.web.config.route.WRouteSubjectManager;
import ru.novosoft.smsc.web.config.sme.WSmeManager;
import ru.novosoft.smsc.web.config.smsc.WSmscManager;
import ru.novosoft.smsc.web.config.snmp.WSnmpManager;
import ru.novosoft.smsc.web.config.timezone.WTimezoneManager;
import ru.novosoft.smsc.web.config.user.WUserManager;
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

  private final AclManager aclManager;
  private final AliasManager aliasManager;
  private final WArchiveDaemonManager archiveDaemonManager;
  private final WCategoryManager categoryManager;
  private final ClosedGroupManager closedGroupManager;
  private final WFraudManager fraudManager;
  private final WLoggerManager loggerManager;
  private final WMapLimitManager mapLimitManager;
  private final WSnmpManager snmpManager;
  private final MscManager mscManager;
  private final ProfileManager profileManager;
  private final WProviderManager providerManager;
  private final WRescheduleManager rescheduleManager;
  private final WRouteSubjectManager routeSubjectManager;
  private final ResourceManager resourceManager;
  private final SmeManager smeManager;
  private final WSmscManager smscManager;
  private final WTimezoneManager timezoneManager;
  private final WUserManager userManager;
  private final OperativeStoreManager operativeStoreManager;
  private final ArchiveDaemon archiveDaemon;
  private final SmscStatProvider smscStatProvider;

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
    aclManager = new WAclManager(adminContext.getAclManager(), journal, user);
    aliasManager = new WAliasManager(adminContext.getAliasManager(), user, journal);
    if (adminContext.getArchiveDaemonManager() != null)
      archiveDaemonManager = new WArchiveDaemonManager(adminContext.getArchiveDaemonManager(), journal, user);
    else
      archiveDaemonManager = null;
    categoryManager = new WCategoryManager(adminContext.getCategoryManager(), journal, user);
    closedGroupManager = new WClosedGroupManager(adminContext.getClosedGroupManager(), user, journal);
    fraudManager = new WFraudManager(adminContext.getFraudManager(), journal, user);
    loggerManager = new WLoggerManager(adminContext.getLoggerManager(), journal, user);
    mapLimitManager = new WMapLimitManager(adminContext.getMapLimitManager(), journal, user);
    snmpManager = new WSnmpManager(adminContext.getSnmpManager(), journal, user);
    mscManager = new WMscManager(adminContext.getMscManager(), journal, user);
    profileManager = new WProfileManager(adminContext.getProfileManager(), journal, user);
    providerManager = new WProviderManager(adminContext.getProviderManager(), journal, user);
    rescheduleManager = new WRescheduleManager(adminContext.getRescheduleManager(), journal, user);
    routeSubjectManager = new WRouteSubjectManager(adminContext.getRouteSubjectManager(), journal, user);
    resourceManager = new WResourceManager(adminContext.getResourceManager(), journal, user);
    smeManager = new WSmeManager(adminContext.getSmeManager(), journal, user);
    smscManager = new WSmscManager(adminContext.getSmscManager(), journal, user);
    timezoneManager = new WTimezoneManager(adminContext.getTimezoneManager(), journal, user);
    userManager = new WUserManager(adminContext.getUsersManager(), journal, user);
    operativeStoreManager = adminContext.getOperativeStoreManager();
    archiveDaemon = adminContext.getArchiveDaemon();
    smscStatProvider = adminContext.getSmscStatProvider();
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

  public AclManager getAclManager() {
    return aclManager;
  }

  public AliasManager getAliasManager() {
    return aliasManager;
  }

  public WArchiveDaemonManager getArchiveDaemonManager() {
    return archiveDaemonManager;
  }

  public WCategoryManager getCategoryManager() {
    return categoryManager;
  }

  public ClosedGroupManager getClosedGroupManager() {
    return closedGroupManager;
  }

  public WFraudManager getFraudManager() {
    return fraudManager;
  }

  public WLoggerManager getLoggerManager() {
    return loggerManager;
  }

  public WMapLimitManager getMapLimitManager() {
    return mapLimitManager;
  }

  public WSnmpManager getSnmpManager() {
    return snmpManager;
  }

  public MscManager getMscManager() {
    return mscManager;
  }

  public ProfileManager getProfileManager() {
    return profileManager;
  }

  public WProviderManager getProviderManager() {
    return providerManager;
  }

  public WRescheduleManager getRescheduleManager() {
    return rescheduleManager;
  }

  public WRouteSubjectManager getRouteSubjectManager() {
    return routeSubjectManager;
  }

  public ResourceManager getResourceManager() {
    return resourceManager;
  }

  public SmeManager getSmeManager() {
    return smeManager;
  }

  public WSmscManager getSmscManager() {
    return smscManager;
  }

  public WTimezoneManager getTimezoneManager() {
    return timezoneManager;
  }

  public WUserManager getUserManager() {
    return userManager;
  }

  public OperativeStoreManager getOperativeStoreManager() {
    return operativeStoreManager;
  }

  public SmscStatProvider getSmscStatProvider() {
    return smscStatProvider;
  }

  public ArchiveDaemon getArchiveDaemon() {
    return archiveDaemon;
  }

  public Journal getJournal() {
    return journal;
  }

}
