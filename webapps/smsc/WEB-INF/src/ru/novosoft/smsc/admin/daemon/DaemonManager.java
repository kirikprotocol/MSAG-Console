/*
 * Created by igork
 * Date: Mar 20, 2002
 * Time: 4:08:25 PM
 */
package ru.novosoft.smsc.admin.daemon;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.smsc_service.SmeManager;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.config.Config;

import java.util.*;
import java.io.IOException;


public class DaemonManager
{
	private DaemonList daemons = new DaemonList();
	private Daemon smscDaemon = null;
	private Category logger = Category.getInstance(this.getClass());
	private Config config = null;

	public DaemonManager(SmeManager smeManager, Config config)
	{
		this.config = config;
		Set daemonNames = config.getSectionChildSectionNames("daemons");
		logger.debug("Initializing daemon manager");
		for (Iterator i = daemonNames.iterator(); i.hasNext();)
		{
			String encodedName = (String) i.next();
			String daemonName = StringEncoderDecoder.decodeDot(encodedName.substring(encodedName.lastIndexOf('.') + 1));
			try
			{
				final int port = config.getInt(encodedName + ".port");
				add(daemonName, port, smeManager);
				logger.debug("Daemon \"" + daemonName + ':' + port + "\" added");
			}
			catch (AdminException e)
			{
				logger.error("Couldn't add daemon \"" + daemonName + "\"", e);
			}
			catch (Config.ParamNotFoundException e)
			{
				logger.debug("Misconfigured daemon \"" + daemonName + "\", parameter port missing", e);
			}
			catch (Config.WrongParamTypeException e)
			{
				logger.debug("Misconfigured daemon \"" + daemonName + "\", parameter port misformatted", e);
			}
		}
		logger.debug("Daemon manager initialized");
	}

	public Daemon add(String host, int port, SmeManager smeManager) throws AdminException
	{
		final Daemon d = new Daemon(host, port, smeManager);
		daemons.add(d);
		try
		{
			findSmscDaemon();
		}
		catch (AdminException e)
		{
			daemons.remove(d.getHost());
			throw e;
		}
		return d;
	}


	public Daemon remove(String host) throws AdminException
	{
		final Daemon daemon = daemons.get(host);
		if (daemon.isContainsSmsc())
		{
			throw new AdminException("Couldn't remove host \"" + host + "\" becouse it is contains SMSC");
		}
		daemons.remove(host);
		findSmscDaemon();
		return daemon;
	}

	public Daemon get(String host) throws AdminException
	{
		return daemons.get(host);
	}

	/**
	 * @return host names
	 */
	public List getHostNames()
	{
		return daemons.getHostNames();
	}

	private void findSmscDaemon() throws AdminException
	{
		smscDaemon = null;
		for (Iterator i = daemons.iterator(); i.hasNext();)
		{
			Daemon daemon = (Daemon) i.next();
			if (daemon.isContainsSmsc())
			{
				if (smscDaemon != null)
					throw new AdminException("more than one daemon contains SMSC");
				else
					smscDaemon = daemon;
			}
		}
	}

	public Daemon getSmscDaemon()
	{
		return smscDaemon;
	}

	public void updateConfig(Config config)
	{
		config.removeSection("daemons");
		for (Iterator i = daemons.iterator(); i.hasNext();)
		{
			Daemon daemon = (Daemon) i.next();
			config.setInt("daemons." + StringEncoderDecoder.encodeDot(daemon.getHost()) + ".port", daemon.getPort());
		}
	}

	public Daemon getServiceDaemon(String serviceId) throws AdminException
	{
		for (Iterator i = daemons.iterator(); i.hasNext();)
		{
			Daemon daemon = (Daemon) i.next();
			if (daemon.isContainsService(serviceId))
				return daemon;
		}
		return null;
	}

	public Map refreshServices(SmeManager smeManager) throws AdminException
	{
		Map result = new HashMap();
		for (Iterator i = daemons.iterator(); i.hasNext();)
		{
			Daemon daemon = (Daemon) i.next();
			result.putAll(daemon.refreshServices(smeManager));
		}
		return result;
	}

	public void addService(ServiceInfo serviceInfo) throws AdminException
	{
		get(serviceInfo.getHost()).addService(serviceInfo);
	}

	public void removeAllServicesFromHost(String host) throws AdminException
	{
		get(host).removeAllServices();
	}

	public void saveHosts() throws AdminException, IOException, Config.WrongParamTypeException
	{
		config.removeSection("daemons");
		for (Iterator i = daemons.getHostNames().iterator(); i.hasNext();) {
			String hostName = (String) i.next();
			config.setInt("daemons." + StringEncoderDecoder.encodeDot(hostName) + ".port", daemons.get(hostName).getPort());
		}
		config.save();
	}
}
