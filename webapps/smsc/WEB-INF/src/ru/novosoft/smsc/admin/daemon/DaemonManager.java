/*
 * Created by igork
 * Date: Mar 20, 2002
 * Time: 4:08:25 PM
 */
package ru.novosoft.smsc.admin.daemon;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.route.SMEList;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.config.Config;

import java.util.*;


public class DaemonManager
{
	private DaemonList daemons = new DaemonList();
	private Daemon smscDaemon = null;
	private Category logger = Category.getInstance(this.getClass());

	public DaemonManager(SMEList smes, Config config)
	{
		Set daemonNames = config.getSectionChildSectionNames("daemons");
		logger.debug("Initializing daemon manager");
		for (Iterator i = daemonNames.iterator(); i.hasNext();)
		{
			String encodedName = (String) i.next();
			String daemonName = StringEncoderDecoder.decodeDot(encodedName.substring(encodedName.lastIndexOf('.') + 1));
			try
			{
				final int port = config.getInt(encodedName + ".port");
				add(daemonName, port, smes);
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

	public Daemon add(String host, int port, SMEList smeList)
			throws AdminException
	{
		final Daemon d = new Daemon(host, port, smeList);
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


	public Daemon remove(String host)
			throws AdminException
	{
		final Daemon daemon = daemons.remove(host);
		findSmscDaemon();
		return daemon;
	}

	public Daemon get(String host)
			throws AdminException
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

	private void findSmscDaemon()
			throws AdminException
	{
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

	public Map refreshServices(SMEList smeList) throws AdminException
	{
		Map result = new HashMap();
		for (Iterator i = daemons.iterator(); i.hasNext();)
		{
			Daemon daemon = (Daemon) i.next();
			result.putAll(daemon.refreshServices(smeList));
		}
		return result;
	}

	public void addService(ServiceInfo serviceInfo) throws AdminException
	{
		Daemon daemon = get(serviceInfo.getHost());
		daemon.addService(serviceInfo);
	}
}
