package ru.novosoft.smsc.admin.service;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.daemon.Daemon;
import ru.novosoft.smsc.admin.daemon.DaemonManager;
import ru.novosoft.smsc.admin.route.SME;
import ru.novosoft.smsc.admin.smsc_service.RouteSubjectManager;
import ru.novosoft.smsc.admin.smsc_service.SmeManager;
import ru.novosoft.smsc.util.config.Config;

import java.io.File;
import java.io.IOException;
import java.util.*;

/**
 * Created by igork
 * Date: Jan 20, 2003
 * Time: 6:23:33 PM
 */
public class HostsManager
{
	private Category logger = Category.getInstance(this.getClass());

	private DaemonManager daemonManager = null;
	private ServiceManager serviceManager = null;
	private SmeManager smeManager = null;
	private RouteSubjectManager routeSubjectManager;
	private long serviceRefreshTimeStamp = 0;

	public HostsManager(DaemonManager daemonManager, ServiceManager serviceManager, SmeManager smeManager, RouteSubjectManager routeSubjectManager)
	{
		this.daemonManager = daemonManager;
		this.serviceManager = serviceManager;
		this.smeManager = smeManager;
		this.routeSubjectManager = routeSubjectManager;
    try {
      refreshServices();
    } catch (AdminException e) {
      logger.error("Couldn't refresh services, skipped", e);
    }
  }


	/* ******************************** daemon operations ***************************************/
	/**
	 * Create and add daemon
	 * @param host
	 * @param port
	 * @return new added daemon
	 * @throws AdminException
	 */
	public synchronized Daemon addHost(String host, int port, String hostServicesFolder) throws AdminException
	{
		final Daemon daemon = daemonManager.add(host, port, smeManager, hostServicesFolder);
		for (Iterator i = daemon.getServiceIds(smeManager).iterator(); i.hasNext();)
		{
			String serviceId = (String) i.next();
			if (serviceManager.contains(serviceId))
			{
				daemonManager.remove(daemon.getHost());
				throw new AdminException("Service \"" + serviceId + "\" from new host \"" + host + ":" + port + "\" already presented in system");
			}
		}
		try
		{
			serviceManager.addAllInfos(daemon.getServices());
		}
		catch (AdminException e)
		{
			daemonManager.remove(daemon.getHost());
			AdminException exception = new AdminException("Couldn't add services from new host \"" + host + "\", host not added. Nested: " + e.getMessage());
			exception.initCause(e);
			throw exception;
		}
		return daemon;
	}

	public synchronized Daemon removeHost(String host) throws AdminException
	{
		logger.debug("Remove host \"" + host + "\"");
		final Daemon daemon = daemonManager.get(host);
		if (daemon.isContainsSmsc())
			throw new AdminException("Couldn't remove host \"" + host + "\": host contains SMSC");
		final List serviceIds = daemon.getServiceIds(smeManager);
		for (Iterator i = serviceIds.iterator(); i.hasNext();)
		{
			String serviceId = (String) i.next();
			logger.debug("  " + serviceId);
		}
		for (Iterator i = serviceIds.iterator(); i.hasNext();)
		{
			String serviceId = (String) i.next();
			if (routeSubjectManager.isSmeUsed(serviceId))
				throw new AdminException("SME \"" + serviceId + "\" is used");
		}
		daemonManager.removeAllServicesFromHost(daemon.getHost());
		serviceManager.removeAll(serviceIds);
		smeManager.removeAllIfSme(serviceIds);
		return daemonManager.remove(daemon.getHost());
	}

	public synchronized List getHostNames()
	{
		return daemonManager.getHostNames();
	}

	/* **************************************** services ************************************************/
	public synchronized List getServiceIds() throws AdminException
	{
		refreshServices();
		return serviceManager.getServiceIds();
	}

	public synchronized Map getServices(String hostName) throws AdminException
	{
		refreshServices();
		return daemonManager.get(hostName).getServices();
	}

	public synchronized Service removeService(String serviceId) throws AdminException
	{
		if (routeSubjectManager.isSmeUsed(serviceId))
			throw new AdminException("Service \"" + serviceId + "\" is used by routes");
		Daemon daemon = daemonManager.getServiceDaemon(serviceId);
		if (daemon == null)
			throw new AdminException("Service \"" + serviceId + "\" host not found");

		daemon.removeService(serviceId);
		Service service = serviceManager.remove(serviceId);
		smeManager.remove(serviceId);

		return service;
	}

	public synchronized long startService(String serviceId) throws AdminException
	{
		try
		{
			ServiceInfo info = serviceManager.getInfo(serviceId);
			Daemon d = daemonManager.get(info.getHost());
			final long pid = d.startService(serviceId);
			info.setPid(pid);
			return pid;
		}
		catch (AdminException e)
		{
			logger.error("Couldn't start service \"" + serviceId + "\"", e);
			throw e;
		}
	}

	public synchronized void killService(String serviceId) throws AdminException
	{
		ServiceInfo info = serviceManager.getInfo(serviceId);
		Daemon d = daemonManager.get(info.getHost());

		d.killService(serviceId);
		info.setPid(0);
	}

	public synchronized void shutdownService(String serviceId) throws AdminException
	{
		daemonManager.get(serviceManager.getInfo(serviceId).getHost()).shutdownService(serviceId);
	}

	public synchronized int getCountRunningServices(String hostName) throws AdminException
	{
		refreshServices();
		return daemonManager.get(hostName).getCountRunningServices();
	}

	public synchronized int getCountServices(String hostName) throws AdminException
	{
		refreshServices();
		return daemonManager.get(hostName).getCountServices();
	}

	public void refreshServices() throws AdminException
	{
		final long currentTime = System.currentTimeMillis();
		if (currentTime - Constants.ServicesRefreshTimeoutMillis > serviceRefreshTimeStamp)
		{
			serviceRefreshTimeStamp = currentTime;
			Map services = daemonManager.refreshServices(smeManager);
			logger.debug("Refresh services: " + services.size() + " services found");
			serviceManager.updateServices(services);
		}
	}

	public synchronized void deployAdministrableService(File incomingZip, ServiceInfo serviceInfo) throws AdminException
	{
		final String id = serviceInfo.getId();
		if (serviceManager.contains(id))
			throw new AdminException("Couldn't add new service \"" + id + "\": service with that ID already contained in system.");
		if (smeManager.contains(id))
			throw new AdminException("Couldn't add new service \"" + id + "\": SME with that ID already contained in system.");
    final File serviceFolder = new File(daemonManager.get(serviceInfo.getHost()).getDaemonServicesFolder(), serviceInfo.getId());
		try
		{
      serviceManager.deployAdministrableService(incomingZip, serviceInfo, serviceFolder);
			daemonManager.addService(serviceInfo);
			smeManager.add(serviceInfo.getSme());
		}
		catch (AdminException e)
		{
			logger.error("Couldn't deploy administrable service \"" + id + "\"", e);
			serviceManager.rollbackDeploy(serviceInfo.getHost(), id, serviceFolder);
			if (smeManager.contains(id))
				smeManager.remove(id);
      throw e;
		}
	}





	/* ***************************************** smes **************************************************/

	public synchronized List getSmeIds()
	{
		return smeManager.getSmeNames();
	}

	public synchronized SME addSme(String id, int priority, byte type, int typeOfNumber, int numberingPlan, int interfaceVersion, String systemType, String password, String addrRange, int smeN, boolean wantAlias, boolean forceDC, int timeout, String receiptSchemeName, boolean disabled, byte mode, int proclimit, int schedlimit) throws AdminException
	{
		return smeManager.add(id, priority, type, typeOfNumber, numberingPlan, interfaceVersion, systemType, password, addrRange, smeN, wantAlias, forceDC, timeout, receiptSchemeName, disabled, mode, proclimit, schedlimit);
	}

	public synchronized void removeSme(String smeId) throws AdminException
	{
		if (serviceManager.contains(smeId))
			throw new AdminException("Couldn't remove sme \"" + smeId + "\" becouse it is service");

		if (isSmeUsed(smeId))
			throw new AdminException("Couldn't remove sme \"" + smeId + "\" becouse it is used by routes");

		smeManager.remove(smeId);
	}


	private boolean isSmeUsed(String smeId)
	{
		return routeSubjectManager.isSmeUsed(smeId);
	}

	public synchronized int getHostPort(String hostName) throws AdminException
	{
		return daemonManager.get(hostName).getPort();
	}

	public synchronized boolean isService(String smeId)
	{
		return serviceManager.contains(smeId);
	}

	public synchronized boolean isServiceAdministarble(String smeId)
	{
		return serviceManager.isServiceAdministarble(smeId);
	}

	public synchronized ServiceInfo getServiceInfo(String serviceId) throws AdminException
	{
		refreshServices();
		return serviceManager.getInfo(serviceId);
	}

  public Service getService(String smeId) throws AdminException
  {
    return serviceManager.get(smeId);
  }

	public synchronized void applyHosts() throws IOException, AdminException, Config.WrongParamTypeException
	{
      daemonManager.saveHosts();
	}

  public synchronized String getDaemonServicesFolder(String hostName) throws AdminException
  {
    return daemonManager.get(hostName).getDaemonServicesFolder();
  }

/*
	public synchronized Daemon getSmscDaemon()
	{
		return daemonManager.getSmscDaemon();
	}
*/
}
