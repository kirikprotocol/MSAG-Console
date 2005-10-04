package ru.sibinco.scag.backend;

import org.apache.log4j.Logger;
import org.xml.sax.SAXException;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.daemon.Daemon;
import ru.sibinco.lib.backend.util.config.Config;
import ru.sibinco.lib.backend.util.conpool.NSConnectionPool;
import ru.sibinco.scag.backend.resources.ResourceManager;
import ru.sibinco.scag.backend.routing.BillingManager;
import ru.sibinco.scag.backend.routing.GwRoutingManager;
import ru.sibinco.scag.backend.sme.*;
import ru.sibinco.scag.backend.users.UserManager;
import ru.sibinco.scag.backend.protocol.journal.Journal;
import ru.sibinco.scag.backend.endpoints.centers.CenterManager;
import ru.sibinco.scag.backend.endpoints.svc.SvcManager;
import ru.sibinco.scag.backend.rules.RuleManager;
import ru.sibinco.scag.perfmon.PerfServer;
import ru.sibinco.tomcat_auth.XmlAuthenticator;

import javax.sql.DataSource;
import javax.xml.parsers.ParserConfigurationException;
import java.io.File;
import java.io.IOException;
import java.sql.SQLException;
import java.util.Properties;


/**
 * Created by IntelliJ IDEA. User: igork Date: 25.02.2004 Time: 17:22:47
 */
public class SCAGAppContext
{
  private static SCAGAppContext instance = null;

  private Logger logger = Logger.getLogger(this.getClass());

  private final Config config;
  private final Config gwConfig;
  private final Config idsConfig;
  private final UserManager userManager;
  private final GwSmeManager gwSmeManager;
  private final CenterManager centerManager;
  private final SvcManager svcManager;
  private final RuleManager ruleManager;
  private final ProviderManager providerManager;
  private final SmscsManager smscsManager;
  private final GwRoutingManager gwRoutingManager;
  private final ResourceManager resourceManager;
  private final PerfServer perfServer;
  private final Daemon gwDaemon;
  private final Gateway gateway;
  private final Statuses statuses;
  private final DataSource connectionPool;
  private final BillingManager billingManager;
  private Journal journal = new Journal();
  private SCAG scag = null;
  protected static File gwConfFolder = null;

  private SCAGAppContext(final String config_filename) throws Throwable, ParserConfigurationException, SAXException, Config.WrongParamTypeException,
                                                                Config.ParamNotFoundException, SibincoException
  {
    try {
      System.out.println("  **  config file:" + new File(config_filename).getAbsolutePath());
      System.out.flush();
      config = new Config(new File(config_filename));
      gwConfig = new Config(new File(config.getString("gw_config")));
      idsConfig = new Config(new File(config.getString("ids_file")));
      String gwDaemonHost=config.getString("gw daemon.host");
      String gwConfigFolder=config.getString("gw_config_folder");
      gwConfFolder=new File(gwConfigFolder);
      connectionPool = null;
      userManager = new UserManager(config.getString("users_config_file"));
      providerManager = new ProviderManager(idsConfig);
      gwSmeManager = new GwSmeManager(config.getString("sme_file"), gwConfig, providerManager);
      gwSmeManager.init();
      //centerManager = new CenterManager(config.getString("centers_file")); //ToDo
      centerManager = new CenterManager(); //ToDo
      centerManager.init();
      //svcManager = new SvcManager(config.getString("services_file"));
      svcManager = new SvcManager();
      svcManager.init();
      String rulesFolder=config.getString("rules_folder");
      String xsdFolder=config.getString("xsd_folder");
      ruleManager=new RuleManager(new File(rulesFolder),new File(xsdFolder),providerManager, idsConfig);
      ruleManager.init();
      smscsManager = new SmscsManager(gwConfig,gwSmeManager);
      resourceManager = new ResourceManager(gwConfFolder);
      scag = new SCAG(gwDaemonHost, (int)config.getInt("gw daemon.port"),gwConfigFolder, this);
      billingManager = new BillingManager(new File(gwConfigFolder, "billing-rules.xml"));
      gwRoutingManager = new GwRoutingManager(gwConfFolder, gwSmeManager, providerManager, billingManager);
      gwRoutingManager.init();
      gwDaemon = new Daemon(gwDaemonHost, (int) config.getInt("gw daemon.port"), gwSmeManager, config.getString("gw daemon.folder"));
      gateway = new Gateway(gwDaemonHost, (int) gwConfig.getInt("admin.port"));
      statuses = new Statuses();
      perfServer = new PerfServer(config);
      perfServer.start();
      XmlAuthenticator.init(new File(config.getString("users_config_file")));
    } catch (Throwable e) {
      logger.fatal("Could not initialize App Context", e);
      e.printStackTrace();
      throw e;
    }
  }

  private static DataSource createConnectionPool(final Config config) throws Config.ParamNotFoundException, Config.WrongParamTypeException, SQLException
  {
    final Properties props = new Properties();
    props.setProperty("jdbc.source", config.getString("jdbc.source"));
    props.setProperty("jdbc.driver", config.getString("jdbc.driver"));
    props.setProperty("jdbc.user", config.getString("jdbc.user"));
    props.setProperty("jdbc.pass", config.getString("jdbc.password"));
    return new NSConnectionPool(props);
  }

  public void destroy()
  {
    perfServer.shutdown();
  }

  public static synchronized SCAGAppContext getInstance(final String config_filename) throws Throwable, IOException, ParserConfigurationException,
                                                                                               Config.ParamNotFoundException, SAXException, SibincoException
  {

    if (null != instance) {
      return instance;
    }
    else {
      instance = new SCAGAppContext(config_filename);
      return instance ;
    }
  }

  public RuleManager getRuleManager()
  {
    return ruleManager;
  }

  public Config getConfig()
  {
    return config;
  }

  public Config getGwConfig()
  {
    return gwConfig;
  }

  public Config getIdsConfig()
  {
    return idsConfig;
  }

  public UserManager getUserManager()
  {
    return userManager;
  }

  public GwSmeManager getGwSmeManager()
  {
    return gwSmeManager;
  }

  public ProviderManager getProviderManager()
  {
    return providerManager;
  }

  public SmscsManager getSmscsManager()
  {
    return smscsManager;
  }

  public GwRoutingManager getGwRoutingManager()
  {
    return gwRoutingManager;
  }

  public ResourceManager getResourceManager()
  {
    return resourceManager;
  }

  public Daemon getGwDaemon()
  {
    return gwDaemon;
  }

  public Gateway getGateway()
  {
    return gateway;
  }

  public Statuses getStatuses()
  {
    return statuses;
  }

  public BillingManager getBillingManager()
  {
    return billingManager;
  }

  public DataSource getDataSource()
  {
    return connectionPool;
  }

  public Journal getJournal()
  {
    return journal;
  }

  public SCAG getSCAG()
  {
    return scag;
  }

    public CenterManager getCenterManager() {
        return centerManager;
    }

    public SvcManager getSvcManager() {
        return svcManager;
    }

    public static File getGwConfFolder() {
        return gwConfFolder;
    }
/*
  public SmeHostsManager getSmeHostsManager()
  {
    return smeHostsManager;
  }

  public HostsManager getHostsManager()
  {
    return hostsManager;
  } */
}
