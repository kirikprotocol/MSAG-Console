package ru.novosoft.smsc.jsp;

import ru.novosoft.smsc.admin.service.ServiceManager;
import ru.novosoft.smsc.admin.smsc_service.Smsc;
import ru.novosoft.smsc.admin.daemon.DaemonManager;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.config.ConfigManager;
import ru.novosoft.util.conpool.NSConnectionPool;
import ru.novosoft.util.jsp.AppContextImpl;

import javax.sql.DataSource;
import java.util.Properties;


public class SMSCAppContextImpl extends AppContextImpl implements SMSCAppContext
{
	ConfigManager configManager = null;
	ServiceManager serviceManager = null;
	DaemonManager daemonManager = null;

	Smsc smsc = null;
	NSConnectionPool connectionPool = null;

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
			props.setProperty("jdbc.user",   configManager.getConfig().getString("profiler.jdbc.user"));
			props.setProperty("jdbc.pass",   configManager.getConfig().getString("profiler.jdbc.password"));
			connectionPool = new NSConnectionPool(props);
			smsc = new Smsc(configManager, connectionPool);
			serviceManager.init(configManager, smsc);
			serviceManager = ServiceManager.getInstance();
			daemonManager = serviceManager.getDaemonManager();
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
}

