package ru.novosoft.smsc.jsp;

import ru.novosoft.smsc.admin.daemon.DaemonManager;
import ru.novosoft.smsc.admin.preferences.UserPreferences;
import ru.novosoft.smsc.admin.service.ServiceManager;
import ru.novosoft.smsc.admin.smsc_service.Smsc;
import ru.novosoft.smsc.admin.users.UserManager;
import ru.novosoft.smsc.util.auth.XmlAuthenticator;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.config.ConfigManager;
import ru.novosoft.util.conpool.NSConnectionPool;
import ru.novosoft.util.jsp.AppContextImpl;

import javax.sql.DataSource;
import java.io.File;
import java.util.Properties;


public class SMSCAppContextImpl extends AppContextImpl implements SMSCAppContext
{
	private ConfigManager configManager = null;
	private ServiceManager serviceManager = null;
	private DaemonManager daemonManager = null;
	private UserManager userManager = null;

	private Smsc smsc = null;
	private NSConnectionPool connectionPool = null;
	private UserPreferences userPreferences = new UserPreferences();
	private Statuses statuses = new StatusesImpl();

	public SMSCAppContextImpl(String configFileName)
	{
		super();
		try
		{
			org.apache.log4j.BasicConfigurator.configure();
			ConfigManager.Init(configFileName);
			configManager = ConfigManager.getInstance();
			Properties props = new Properties();
			props.setProperty("jdbc.source", configManager.getConfig().getString("profiler.jdbc.source"));
			props.setProperty("jdbc.driver", configManager.getConfig().getString("profiler.jdbc.driver"));
			props.setProperty("jdbc.user", configManager.getConfig().getString("profiler.jdbc.user"));
			props.setProperty("jdbc.pass", configManager.getConfig().getString("profiler.jdbc.password"));
			connectionPool = new NSConnectionPool(props);
			smsc = new Smsc(configManager, connectionPool);
			serviceManager.init(configManager, smsc);
			serviceManager = ServiceManager.getInstance();
			daemonManager = serviceManager.getDaemonManager();
			System.out.println("SMSCAppContextImpl.SMSCAppContextImpl **************************************************");
			File usersConfig = new File(new File(configManager.getConfig().getString("system.webapp folder"), "WEB-INF"), configManager.getConfig().getString("system.users"));
			userManager = new UserManager(usersConfig);
		}
		catch (Exception e)
		{
			System.out.println("Exception in initialization:");
			e.printStackTrace();
		}
	}

	public Config getConfig()
	{
		return configManager.getConfig();
	}

	public ServiceManager getServiceManager()
	{
		return serviceManager;
	}

	public Smsc getSmsc()
	{
		return smsc;
	}

	public DataSource getConnectionPool()
	{
		return connectionPool;
	}

	public DaemonManager getDaemonManager()
	{
		return daemonManager;
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
}

