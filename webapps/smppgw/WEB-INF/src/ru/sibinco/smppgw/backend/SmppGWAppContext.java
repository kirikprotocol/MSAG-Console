package ru.sibinco.smppgw.backend;

import org.apache.log4j.Logger;
import org.xml.sax.SAXException;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.daemon.Daemon;
import ru.sibinco.lib.backend.daemon.ServiceInfo;
import ru.sibinco.lib.backend.util.config.Config;
import ru.sibinco.smppgw.backend.resources.ResourceManager;
import ru.sibinco.smppgw.backend.routing.GwRoutingManager;
import ru.sibinco.smppgw.backend.sme.*;
import ru.sibinco.smppgw.backend.users.UserManager;
import ru.sibinco.tomcat_auth.XmlAuthenticator;

import javax.xml.parsers.ParserConfigurationException;
import java.io.File;
import java.io.IOException;


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
  private final Daemon gwDaemon;
  private final Gateway gateway;
  private final Statuses statuses;

  private SmppGWAppContext(final String config_filename) throws Throwable, ParserConfigurationException, SAXException, Config.WrongParamTypeException,
                                                                Config.ParamNotFoundException, SibincoException
  {
    try {
      System.out.println("  **  config file:" + new File(config_filename).getAbsolutePath());
      System.out.flush();
      config = new Config(new File(config_filename));
      gwConfig = new Config(new File(config.getString("gw_config")));
      userManager = new UserManager(config.getString("users_config_file"));
      gwSmeManager = new GwSmeManager(config.getString("sme_file"), gwConfig);
      providerManager = new ProviderManager(gwConfig);
      smscsManager = new SmscsManager(gwConfig);
      gwRoutingManager = new GwRoutingManager(new File(config.getString("gw_config_folder")), gwSmeManager, providerManager);
      gwRoutingManager.init();
      resourceManager = new ResourceManager(new File(config.getString("gw_config_folder")));
      gwDaemon = new Daemon(config.getString("gw daemon.host"), (int) config.getInt("gw daemon.port"), gwSmeManager, config.getString("gw daemon.folder"));
      final ServiceInfo gwServiceInfo = (ServiceInfo) gwDaemon.getServices().get(config.getString("gw name"));
      gateway = new Gateway(gwServiceInfo);
      statuses = new Statuses();
      XmlAuthenticator.init(new File(config.getString("users_config_file")));
    } catch (Throwable e) {
      logger.fatal("Could not initialize App Context", e);
      throw e;
    }
  }

  public static synchronized SmppGWAppContext getInstance(final String config_filename) throws Throwable, IOException, ParserConfigurationException,
                                                                                               Config.ParamNotFoundException, SAXException, SibincoException
  {
    return instance != null
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
}
