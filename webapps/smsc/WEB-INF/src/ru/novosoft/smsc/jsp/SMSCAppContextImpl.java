package ru.novosoft.smsc.jsp;

import ru.novosoft.smsc.admin.daemon.DaemonManager;
import ru.novosoft.smsc.admin.preferences.UserPreferences;
import ru.novosoft.smsc.admin.service.*;
import ru.novosoft.smsc.admin.smsc_service.*;
import ru.novosoft.smsc.admin.users.UserManager;
import ru.novosoft.smsc.admin.console.Console;
import ru.novosoft.smsc.admin.route.RouteSubjectManagerImpl;
import ru.novosoft.smsc.admin.resources.ResourcesManager;
import ru.novosoft.smsc.admin.resources.ResourcesManagerImpl;
import ru.novosoft.smsc.admin.journal.Journal;
import ru.novosoft.smsc.perfmon.PerfServer;
import ru.novosoft.smsc.util.*;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.util.conpool.NSConnectionPool;
import ru.novosoft.util.jsp.AppContextImpl;

import javax.sql.DataSource;
import java.io.*;
import java.sql.SQLException;
import java.util.*;

import org.apache.log4j.PropertyConfigurator;
import org.apache.log4j.BasicConfigurator;


public class SMSCAppContextImpl extends AppContextImpl implements SMSCAppContext
{
	private Config smscConfig = null;
	private HostsManager hostsManager = null;
	private UserManager userManager = null;
	private PerfServer perfServer = null;
	private SmeManager smeManager = null;
	private RouteSubjectManager routeSubjectManager = null;
	private ResourcesManager resourcesManager = null;

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
		else
		{
			Properties props = new Properties();
			try
			{
				props.load(is);
			}
			catch (IOException e)
			{
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
		if (!log4jinit.exists())
		{
			InputStream is = this.getClass().getClassLoader().getResourceAsStream("log4j.properties");
			if (is == null)
			{
				System.out.println("initializing log4j by BasicConfigurator");
				BasicConfigurator.configure();
			}
			else
			{
				System.out.println("initializing log4j by resource log4j.properties");
				initLoggerByProps(is);
			}
		}
		else
		{
			try
			{
				System.out.println("initializing log4j by external file: " + log4jinit.getAbsolutePath());
				initLoggerByProps(new FileInputStream(log4jinit));
			}
			catch (FileNotFoundException e)
			{
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
		try
		{
			System.out.println("Starting SMSC Administartion Web Apllication **************************************************");
			initLogger();
			smscConfig = new Config(new File(configFileName));
			WebAppFolders.init(smscConfig.getString("system.webapp folder"), smscConfig.getString("system.work folder"), smscConfig.getString("smsc.config folder"));

			resourcesManager = new ResourcesManagerImpl();
      loadLocaleMessages();
			createConnectionPool(smscConfig);

			smsc = new Smsc(smscConfig.getString("smsc.host"), smscConfig.getInt("smsc.port"), smscConfig.getString("smsc.config folder"), connectionPool);
			smeManager = new SmeManagerImpl(smsc);
			routeSubjectManager = new RouteSubjectManagerImpl(smeManager);
      DaemonManager daemonManager = new DaemonManager(smeManager, smscConfig);
			ServiceManagerImpl serviceManager = new ServiceManagerImpl();
			serviceManager.add(smsc);
			hostsManager = new HostsManager(daemonManager, serviceManager, smeManager, routeSubjectManager);

			File usersConfig = new File(smscConfig.getString("system.users file"));
			startConsole();
			userManager = new UserManager(usersConfig);
      statuses.setRoutesSaved(routeSubjectManager.hasSavedConfiguration());
			perfServer = new PerfServer(smscConfig);
			perfServer.start();
			System.out.println("SMSC Administartion Web Apllication Started  **************************************************");
		}
		catch (Exception e)
		{
			System.out.println("Exception in initialization:");
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
		try
		{
			needConsole = smscConfig.getBool("console.enabled");
		}
		catch (Exception eee)
		{ /* do nothing, console disabled by default */
		}
		if (needConsole)
		{
			console = new Console(this);
			console.start();
		}
	}

	public Config getConfig()
	{
		return smscConfig;
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
			synchronized (userPreferences)
			{
				UserPreferences prefs = (UserPreferences) userPreferences.get(loginedPrincipal);
				if (prefs == null)
				{
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
		perfServer.shutdown();
	}

	public String getLocaleString(Locale locale, String key)
	{
		return localeMessages.getString(locale, key);
	}

  public Journal getJournal()
  {
    return journal;
  }
}

