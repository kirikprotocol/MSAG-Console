package ru.novosoft.smsc.jsp;

import org.apache.log4j.BasicConfigurator;
import org.apache.log4j.PropertyConfigurator;
import ru.novosoft.smsc.admin.acl.AclManager;
import ru.novosoft.smsc.admin.console.Console;
import ru.novosoft.smsc.admin.daemon.DaemonManager;
import ru.novosoft.smsc.admin.journal.Journal;
import ru.novosoft.smsc.admin.preferences.UserPreferences;
import ru.novosoft.smsc.admin.resources.ResourcesManager;
import ru.novosoft.smsc.admin.resources.ResourcesManagerImpl;
import ru.novosoft.smsc.admin.route.RouteSubjectManagerImpl;
import ru.novosoft.smsc.admin.service.HostsManager;
import ru.novosoft.smsc.admin.service.ServiceManagerImpl;
import ru.novosoft.smsc.admin.smsc_service.*;
import ru.novosoft.smsc.admin.users.UserManager;
import ru.novosoft.smsc.perfmon.PerfServer;
import ru.novosoft.smsc.topmon.TopServer;
import ru.novosoft.smsc.util.LocaleMessages;
import ru.novosoft.smsc.util.WebAppFolders;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.xml.WebXml;
import ru.novosoft.util.conpool.NSConnectionPool;
import ru.novosoft.util.jsp.AppContextImpl;

import javax.sql.DataSource;
import java.io.*;
import java.sql.SQLException;
import java.util.*;


public class SMSCAppContextImpl extends AppContextImpl implements SMSCAppContext
{
  private Config webappConfig = null;
  private WebXml webXmlConfig = null;
  private HostsManager hostsManager = null;
  private UserManager userManager = null;
  private PerfServer perfServer = null;
  private TopServer topServer = null;
  private SmeManager smeManager = null;
  private RouteSubjectManager routeSubjectManager = null;
  private ResourcesManager resourcesManager = null;
  private AclManager aclManager = null;

  private Smsc smsc = null;
  private NSConnectionPool connectionPool = null;
  private Map userPreferences = new HashMap();
  private Statuses statuses = new StatusesImpl();
  private LocaleMessages localeMessages = null;
  private Journal journal = new Journal();

  private Console console = null;

  private void initLoggerByProps(InputStream is)
  {
    if (is == null)
      BasicConfigurator.configure();
    else {
      Properties props = new Properties();
      try {
        props.load(is);
      } catch (IOException e) {
        e.printStackTrace(System.err);
        System.out.println("Couldn't read log4j init file, configure by BasicConfigurator");
        e.printStackTrace(System.out);
        BasicConfigurator.configure();
      }
      PropertyConfigurator.configure(props);
    }
  }

  private void initLogger()
  {
    System.out.println("initializing logger");
    File log4jinit = new File("conf/log4j.properties");
    if (!log4jinit.exists())
      log4jinit = new File("../conf/log4j.properties");
    if (!log4jinit.exists())
      log4jinit = new File("log4j.properties");
    if (!log4jinit.exists()) {
      InputStream is = this.getClass().getClassLoader().getResourceAsStream("log4j.properties");
      if (is == null) {
        System.out.println("initializing log4j by BasicConfigurator");
        BasicConfigurator.configure();
      } else {
        System.out.println("initializing log4j by resource log4j.properties");
        initLoggerByProps(is);
      }
    } else {
      try {
        System.out.println("initializing log4j by external file: " + log4jinit.getAbsolutePath());
        initLoggerByProps(new FileInputStream(log4jinit));
      } catch (FileNotFoundException e) {
        e.printStackTrace(System.err);
        System.out.println("Couldn't read log4j init file, configure by BasicConfigurator");
        e.printStackTrace(System.out);
        BasicConfigurator.configure();
      }
    }
  }

  public SMSCAppContextImpl(String configFileName)
  {
    super();
    try {
      System.out.println("Starting SMSC Administartion Web Apllication **************************************************");
      initLogger();
      webappConfig = new Config(new File(configFileName));
      WebAppFolders.init(webappConfig.getString("system.webapp folder"), webappConfig.getString("system.work folder"), webappConfig.getString("smsc.config folder"));

      try {
        webXmlConfig = new WebXml(new File(WebAppFolders.getWebinfFolder(), "web.xml"));
      } catch (Throwable e) {
        System.err.println("Could not load web.xml - administration services access rights can not be changed");
        e.printStackTrace();
      }

      resourcesManager = new ResourcesManagerImpl();
      loadLocaleMessages();
      createConnectionPool(webappConfig);

      smsc = new Smsc(webappConfig.getString("smsc.host"), webappConfig.getInt("smsc.port"), webappConfig.getString("smsc.config folder"), connectionPool);
      smeManager = new SmeManagerImpl(smsc);
      routeSubjectManager = new RouteSubjectManagerImpl(smeManager);
      DaemonManager daemonManager = new DaemonManager(smeManager, webappConfig);
      ServiceManagerImpl serviceManager = new ServiceManagerImpl();
      serviceManager.add(smsc);
      hostsManager = new HostsManager(daemonManager, serviceManager, smeManager, routeSubjectManager);
      aclManager = new AclManager(this);

      File usersConfig = new File(webappConfig.getString("system.users file"));
      startConsole();
      userManager = new UserManager(usersConfig);
      statuses.setRoutesSaved(routeSubjectManager.hasSavedConfiguration());
      perfServer = new PerfServer(webappConfig);
      perfServer.start();
      topServer = new TopServer(webappConfig);
      topServer.start();
      System.out.println("SMSC Administartion Web Apllication Started  **************************************************");
    } catch (Exception e) {
      System.err.println("Exception in initialization:");
      e.printStackTrace();
    }
  }

  private void createConnectionPool(Config config) throws Config.ParamNotFoundException, Config.WrongParamTypeException, SQLException
  {
    Properties props = new Properties();
    props.setProperty("jdbc.source", config.getString("profiler.jdbc.source"));
    props.setProperty("jdbc.driver", config.getString("profiler.jdbc.driver"));
    props.setProperty("jdbc.user", config.getString("profiler.jdbc.user"));
    props.setProperty("jdbc.pass", config.getString("profiler.jdbc.password"));
    connectionPool = new NSConnectionPool(props);
  }

  private void loadLocaleMessages()
  {
    localeMessages = new LocaleMessages();
  }

  private void startConsole() throws Exception
  {
    boolean needConsole = false;
    try {
      needConsole = webappConfig.getBool("console.enabled");
    } catch (Exception eee) { /* do nothing, console disabled by default */
    }
    if (needConsole) {
      console = new Console(this);
      console.start();
    }
  }

  public Config getConfig()
  {
    return webappConfig;
  }

  public HostsManager getHostsManager()
  {
    return hostsManager;
  }

  public Smsc getSmsc()
  {
    return smsc;
  }

  public SmeManager getSmeManager()
  {
    return smeManager;
  }

  public RouteSubjectManager getRouteSubjectManager()
  {
    return routeSubjectManager;
  }

  public ResourcesManager getResourcesManager()
  {
    return resourcesManager;
  }

  public DataSource getConnectionPool()
  {
    return connectionPool;
  }

  public UserPreferences getUserPreferences(java.security.Principal loginedPrincipal)
  {
    if (loginedPrincipal == null)
      return new UserPreferences();
    else
      synchronized (userPreferences) {
        UserPreferences prefs = (UserPreferences) userPreferences.get(loginedPrincipal);
        if (prefs == null) {
          prefs = new UserPreferences();
          userPreferences.put(loginedPrincipal, prefs);
        }
        return prefs;
      }
  }

  public Statuses getStatuses()
  {
    return statuses;
  }

  public UserManager getUserManager()
  {
    return userManager;
  }

  public void destroy()
  {
    if (console != null) console.close();
    if (perfServer != null) perfServer.shutdown();
  }

  public String getLocaleString(Locale locale, String key)
  {
    return localeMessages.getString(locale, key);
  }

  public Set getLocaleStrings(Locale locale, String prefix)
  {
    return localeMessages.getStrings(locale, prefix);
  }

  public Journal getJournal()
  {
    return journal;
  }

  public WebXml getWebXmlConfig()
  {
    return webXmlConfig;
  }

  public AclManager getAclManager()
  {
    return aclManager;
  }
}

