package ru.sibinco.smppgw.backend;

import org.apache.log4j.Logger;
import org.xml.sax.SAXException;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.daemon.Daemon;
import ru.sibinco.lib.backend.daemon.ServiceInfo;
import ru.sibinco.lib.backend.util.config.Config;
import ru.sibinco.lib.backend.util.conpool.NSConnectionPool;
import ru.sibinco.smppgw.backend.resources.ResourceManager;
import ru.sibinco.smppgw.backend.routing.BillingManager;
import ru.sibinco.smppgw.backend.routing.GwRoutingManager;
import ru.sibinco.smppgw.backend.sme.*;
import ru.sibinco.smppgw.backend.users.UserManager;
import ru.sibinco.smppgw.perfmon.PerfServer;
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
public class SmppGWAppContext
{
  private static SmppGWAppContext instance = null;

  private Logger logger = Logger.getLogger(this.getClass());

  private final Config config;
  private final Config gwConfig;
  private final UserManager userManager;
  private final GwSmeManager gwSmeManager;
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

  private SmppGWAppContext(final String config_filename) throws Throwable, ParserConfigurationException, SAXException, Config.WrongParamTypeException,
                                                                Config.ParamNotFoundException, SibincoException
  {
    try {
      System.out.println("  **  config file:" + new File(config_filename).getAbsolutePath());
      System.out.flush();
      config = new Config(new File(config_filename));
      connectionPool = createConnectionPool(config);
      gwConfig = new Config(new File(config.getString("gw_config")));
      userManager = new UserManager(config.getString("users_config_file"));
      providerManager = new ProviderManager(gwConfig);
      gwSmeManager = new GwSmeManager(config.getString("sme_file"), gwConfig, providerManager);
      gwSmeManager.init();
      smscsManager = new SmscsManager(gwConfig);
      billingManager = new BillingManager(new File(config.getString("gw_config_folder"), "billing-rules.xml"));
      gwRoutingManager = new GwRoutingManager(new File(config.getString("gw_config_folder")), gwSmeManager, providerManager, billingManager);
      gwRoutingManager.init();
      resourceManager = new ResourceManager(new File(config.getString("gw_config_folder")));
      gwDaemon = new Daemon(config.getString("gw daemon.host"), (int) config.getInt("gw daemon.port"), gwSmeManager, config.getString("gw daemon.folder"));
      final ServiceInfo gwServiceInfo = (ServiceInfo) gwDaemon.getServices().get(config.getString("gw name"));
      gateway = new Gateway(gwServiceInfo, (int) gwConfig.getInt("admin.port"));
      statuses = new Statuses();
      perfServer = new PerfServer(config);
      perfServer.start();
      XmlAuthenticator.init(new File(config.getString("users_config_file")));
    } catch (Throwable e) {
      logger.fatal("Could not initialize App Context", e);
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

  public static synchronized SmppGWAppContext getInstance(final String config_filename) throws Throwable, IOException, ParserConfigurationException,
                                                                                               Config.ParamNotFoundException, SAXException, SibincoException
  {
    return null != instance
           ? instance
           : (instance = new SmppGWAppContext(config_filename));
  }

  public Config getConfig()
  {
    return config;
  }

  public Config getGwConfig()
  {
    return gwConfig;
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
}
