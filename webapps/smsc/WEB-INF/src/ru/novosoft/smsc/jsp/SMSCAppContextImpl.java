package ru.novosoft.smsc.jsp;

import ru.novosoft.smsc.admin.daemon.DaemonManager;
import ru.novosoft.smsc.admin.preferences.UserPreferences;
import ru.novosoft.smsc.admin.service.*;
import ru.novosoft.smsc.admin.smsc_service.Smsc;
import ru.novosoft.smsc.admin.users.UserManager;
import ru.novosoft.smsc.admin.console.Console;
import ru.novosoft.smsc.perfmon.PerfServer;
import ru.novosoft.smsc.util.WebAppFolders;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.config.ConfigManager;
import ru.novosoft.util.conpool.NSConnectionPool;
import ru.novosoft.util.jsp.AppContextImpl;

import javax.sql.DataSource;
import java.io.File;
import java.sql.SQLException;
import java.util.*;


public class SMSCAppContextImpl extends AppContextImpl implements SMSCAppContext
{
	private ConfigManager configManager = null;
	private HostsManager hostsManager = null;
	private UserManager userManager = null;
	private PerfServer perfServer = null;

	private Smsc smsc = null;
	private NSConnectionPool connectionPool = null;
	private UserPreferences userPreferences = new UserPreferences();
	private Statuses statuses = new StatusesImpl();
	private Map localeMessages = new HashMap();

	private Console console = null;

	public SMSCAppContextImpl(String configFileName)
	{
		super();
		try
		{
			System.out.println("Starting SMSC Administartion Web Apllication **************************************************");
			org.apache.log4j.BasicConfigurator.configure();
			ConfigManager.Init(configFileName);
			configManager = ConfigManager.getInstance();
			final Config config = configManager.getConfig();
			WebAppFolders.init(config.getString("system.webapp folder"));

			loadLocaleMessages();
			createConnectionPool(config);

			smsc = new Smsc(configManager, connectionPool);
			DaemonManager daemonManager = new DaemonManager(smsc.getSmes(), config);
			ServiceManager serviceManager = new ServiceManagerImpl();
			hostsManager = new HostsManager(daemonManager, serviceManager, smsc);

			File usersConfig = new File(new File(config.getString("system.webapp folder"), "WEB-INF"), config.getString("system.users"));
			startConsole();
			userManager = new UserManager(usersConfig);
			perfServer = new PerfServer(config);
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
		Locale locale = new Locale("ru");
		localeMessages.put(locale, ResourceBundle.getBundle("locales.messages", locale));
		locale = new Locale("en");
		localeMessages.put(locale, ResourceBundle.getBundle("locales.messages", locale));
	}

	private void startConsole()
			throws Exception
	{
		boolean needConsole = false;
		try
		{
			needConsole = configManager.getConfig().getBool("console.enabled");
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
		return configManager.getConfig();
	}

	public HostsManager getHostsManager()
	{
		return hostsManager;
	}

	public Smsc getSmsc()
	{
		return smsc;
	}

	public DataSource getConnectionPool()
	{
		return connectionPool;
	}

	public UserPreferences getUserPreferences()
	{
		return userPreferences;
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

	public ResourceBundle getLocaleMessages(Locale locale)
	{
		return (ResourceBundle) localeMessages.get(locale);
	}
}

