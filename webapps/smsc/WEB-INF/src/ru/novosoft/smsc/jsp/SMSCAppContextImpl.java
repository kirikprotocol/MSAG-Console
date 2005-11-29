package ru.novosoft.smsc.jsp;

import org.apache.log4j.BasicConfigurator;
import org.apache.log4j.PropertyConfigurator;
import ru.novosoft.smsc.admin.acl.AclManager;
import ru.novosoft.smsc.admin.category.CategoryManager;
import ru.novosoft.smsc.admin.console.Console;
import ru.novosoft.smsc.admin.journal.Journal;
import ru.novosoft.smsc.admin.provider.ProviderManager;
import ru.novosoft.smsc.admin.resources.ResourcesManager;
import ru.novosoft.smsc.admin.resources.ResourcesManagerImpl;
import ru.novosoft.smsc.admin.route.RouteSubjectManagerImpl;
import ru.novosoft.smsc.admin.service.HostsManager;
import ru.novosoft.smsc.admin.service.ServiceManagerImpl;
import ru.novosoft.smsc.admin.service.ServiceManager;
import ru.novosoft.smsc.admin.smsc_service.*;
import ru.novosoft.smsc.admin.users.UserManager;
import ru.novosoft.smsc.admin.users.User;
import ru.novosoft.smsc.admin.resource_group.ResourceGroupManager;
import ru.novosoft.smsc.admin.resource_group.ResourceGroupConstants;
import ru.novosoft.smsc.admin.AdminException;
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

//import java.security.Principal;


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
	private ProviderManager providerManager = null;
	private CategoryManager categoryManager = null;
	private AclManager aclManager = null;
	private ServiceManager serviceManager = null;

	private Smsc smsc = null;
	private SmscList smscList = null;
	private NSConnectionPool connectionPool = null;

	private Statuses statuses = new StatusesImpl();
	private Journal journal = new Journal();

	private Console console = null;

	private String initErrorCode = null;
	private byte instType = ResourceGroupConstants.RESOURCEGROUP_TYPE_UNKNOWN;

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
      }
      else {
        System.out.println("initializing log4j by resource log4j.properties");
        initLoggerByProps(is);
      }
    }
    else {
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
      System.out.println("Starting SMSC Administartion Web Application **************************************************");
      initLogger();
      webappConfig = new Config(new File(configFileName));
      System.out.println("webappConfig = " + configFileName + " **************************************************");
      WebAppFolders.init(webappConfig.getString("system.webapp folder"), webappConfig.getString("system.work folder"));

      try {
        webXmlConfig = new WebXml(new File(WebAppFolders.getWebinfFolder(), "web.xml"));
      } catch (Throwable e) {
        System.err.println("Could not load web.xml - administration services access rights can not be changed");
        e.printStackTrace();
      }

      resourcesManager = new ResourcesManagerImpl();
      createConnectionPool(webappConfig);

		serviceManager = new ServiceManagerImpl();
		smscList = new SmscList(webappConfig, connectionPool, this);
      smeManager = new SmeManagerImpl(smsc);
      routeSubjectManager = new RouteSubjectManagerImpl(smeManager);
		try
		{
			this.instType = ResourceGroupConstants.getTypeFromString(webappConfig.getString(ResourceGroupConstants.RESOURCEGROUP_INSTALLTYPE_PARAM_NAME));
		}
		catch (Exception e)
		{
			System.out.println("Installation type is not defined in webconfig");
			throw new AdminException("Installation type is not defined in webconfig");
		}
      ResourceGroupManager resourceGroupManager = new ResourceGroupManager(this);
      hostsManager = new HostsManager(resourceGroupManager, serviceManager, smeManager, routeSubjectManager);
      aclManager = new AclManager(this);
		Config localeConfig = new Config(new File(webappConfig.getString("system.localization file")));
		LocaleMessages.init(localeConfig); // должно вызыватьс€ раньше, чем ёзерћанагер
		File usersConfig = new File(webappConfig.getString("system.users file"));
		userManager = new UserManager(usersConfig);
      providerManager = new ProviderManager(webappConfig);
      categoryManager = new CategoryManager(webappConfig);
      getStatuses().setRoutesSaved(routeSubjectManager.hasSavedConfiguration());
      perfServer = new PerfServer(webappConfig);
		perfServer.setSmscHost(smscList.getSmsc().getHost());
      perfServer.start();
      topServer = new TopServer(webappConfig);
		topServer.setSmscHost(smscList.getSmsc().getHost());
      topServer.start();
      startConsole();
      System.out.println("SMSC Administartion Web Application Started  **************************************************");
    }
	catch (Exception e)
	{
		System.err.println("Exception in initialization:");
		e.printStackTrace();
		initErrorCode = SMSCErrors.error.appContextNotInitialized;
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

	public void setSmsc(Smsc newSmsc)
	{
		smsc = newSmsc;
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

  public ProviderManager getProviderManager()
  {
    return providerManager;
  }

  public CategoryManager getCategoryManager()
  {
    return categoryManager;
  }

  public DataSource getConnectionPool()
  {
    return connectionPool;
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
    if (topServer != null) topServer.shutdown();
  }

	public Locale getLocale()
	{
		Locale result = null;
		if (userManager != null)
		{
			User user = userManager.getLoginedUser();
			if (user != null)	result = user.getPrefs().getLocale();
		}
		if (result == null) result = new Locale(LocaleMessages.DEFAULT_PREFERRED_LANGUAGE);
		return result;
	}

	public String getLocaleString(String key)
	{
		String result = null;
		if (userManager != null) // на вс€кий случай
		{
			User user = userManager.getLoginedUser();
			if (user != null) result = getLocaleString(user.getPrefs().getLocale(),key);
		}
		if (result == null)
		{
			try
			{
				ResourceBundle bundle = ResourceBundle.getBundle(LocaleMessages.SMSC_BUNDLE_NAME, new Locale(LocaleMessages.DEFAULT_PREFERRED_LANGUAGE));
				result = bundle.getString(key);
			}
			catch (MissingResourceException e)
			{
				result = "ResourceNotFound: " + key;
			}

		}
		return result;
	}

	private String getLocaleString(Locale locale, String key)
	{
		return LocaleMessages.getString(locale, key);
	}

	public Set getLocaleStrings(String prefix)
	{
		return getLocaleStrings(userManager.getLoginedUser().getPrefs().getLocale(), prefix);
	}


	private Set getLocaleStrings(Locale locale, String prefix)
	{
		return LocaleMessages.getStrings(locale, prefix);
	}

	public String getInitErrorCode()
	{
		return initErrorCode;
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

	public SmscList getSmscList()
	{
		return smscList;
	}

	public ServiceManager getServiceManager()
	{
		return serviceManager;
	}

	public void setTopMonSmscHost(String smscHost)
	{
		topServer.setSmscHost(smscHost);
	}

	public void setPerfMonSmscHost(String smscHost)
	{
		perfServer.setSmscHost(smscHost);
	}

	public byte getInstallType()
	{
		return instType;
	}
}

