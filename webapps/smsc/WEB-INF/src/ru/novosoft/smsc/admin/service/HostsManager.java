package ru.novosoft.smsc.admin.service;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.daemon.Daemon;
import ru.novosoft.smsc.admin.daemon.DaemonManager;
import ru.novosoft.smsc.admin.route.SME;
import ru.novosoft.smsc.admin.smsc_service.SmeManager;

import java.io.File;
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
	private long serviceRefreshTimeStamp = 0;

	public HostsManager(DaemonManager daemonManager, ServiceManager serviceManager, SmeManager smeManager)
	{
		this.daemonManager = daemonManager;
		this.serviceManager = serviceManager;
		this.smeManager = smeManager;
	}


	/* ******************************** daemon operations ***************************************/
	/**
	 * Create and add daemon
	 * @param host
	 * @param port
	 * @return new added daemon
	 * @throws AdminException
	 */
	public Daemon addHost(String host, int port) throws AdminException
	{ //? add smes
		final Daemon daemon = daemonManager.add(host, port, smeManager.getSmes());
		for (Iterator i = daemon.getServiceIds(smeManager.getSmes()).iterator(); i.hasNext();)
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
			serviceManager.addAll(daemon.getServices());
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

	public Daemon removeHost(String host) throws AdminException
	{
		final Daemon daemon = daemonManager.get(host);
		final List serviceIds = daemon.getServiceIds(smeManager.getSmes());
		for (Iterator i = serviceIds.iterator(); i.hasNext();)
		{
			String serviceId = (String) i.next();
			if (smeManager.isUsed(serviceId))
				throw new AdminException("SME \"" + serviceId + "\" is used");
		}
		serviceManager.removeAll(serviceIds);
		smeManager.removeAllIfSme(serviceIds);
		return daemonManager.remove(daemon.getHost());
	}

	public List getHostNames()
	{
		return daemonManager.getHostNames();
	}

	/* **************************************** services ************************************************/
	public List getServiceIds() throws AdminException
	{
		refreshServices();
		return serviceManager.getServiceIds();
	}

	public Map getServices(String hostName) throws AdminException
	{
		refreshServices();
		return daemonManager.get(hostName).getServices();
	}

	public Service removeService(String serviceId) throws AdminException
	{
		if (smeManager.isUsed(serviceId))
			throw new AdminException("Service \"" + serviceId + "\" is used");
		Daemon daemon = daemonManager.getServiceDaemon(serviceId);
		if (daemon == null)
			throw new AdminException("Service \"" + serviceId + "\" host not found");

		daemon.removeService(serviceId);
		Service service = serviceManager.remove(serviceId);
		smeManager.remove(serviceId);

		return service;
	}

	public long startService(String serviceId)
			throws AdminException
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

	public void killService(String serviceId)
			throws AdminException
	{
		ServiceInfo info = serviceManager.getInfo(serviceId);
		Daemon d = daemonManager.get(info.getHost());

		d.killService(serviceId);
		info.setPid(0);
	}

	public void shutdownService(String serviceId)
			throws AdminException
	{
		daemonManager.get(serviceManager.getInfo(serviceId).getHost()).shutdownService(serviceId);
	}

	protected Object callServiceMethod(String hostName, String serviceId, String componentName,
												  String methodName, String returnTypeName, Map args)
			throws AdminException
	{//toremove
		Service s = serviceManager.get(serviceId);
		if (!s.getInfo().getHost().equals(hostName))
			throw new AdminException("Wrong host name (\"" + hostName + "\")");
		Component c = (Component) s.getInfo().getComponents().get(componentName);
		if (c == null)
			throw new AdminException("Wrong component name (\"" + componentName + "\")");
		Method m = (Method) c.getMethods().get(methodName);
		if (m == null)
			throw new AdminException("Wrong method name (\"" + methodName + "\")");
		Type t = Type.getInstance(returnTypeName);
		if (!m.getType().equals(t))
			throw new AdminException("Wrong method return type (\"" + returnTypeName + "\")");
		Map params = m.getParams();
		if (!params.keySet().equals(args.keySet()))
			throw new AdminException("Wrong arguments");
		Map arguments = new HashMap();
		for (Iterator i = params.values().iterator(); i.hasNext();)
		{
			Parameter p = (Parameter) i.next();
			if (args.get(p.getName()) == null)
				throw new AdminException("Parameter \"" + p.getName() + "\" not specified");
			switch (p.getType().getId())
			{
				case Type.StringType:
					{
						arguments.put(p.getName(), args.get(p.getName()));
						break;
					}
				case Type.IntType:
					{
						arguments.put(p.getName(), Integer.decode((String) args.get(p.getName())));
						break;
					}
				case Type.BooleanType:
					{
						arguments.put(p.getName(), Boolean.valueOf((String) args.get(p.getName())));
						break;
					}
				default:
					{
						throw new AdminException("Unknown parameter \"" + p.getName() + "\" type \"" + p.getType().getName() + "\"");
					}
			}
		}
		return s.call(c, m, t, arguments);
	}

	public int getCountRunningServices(String hostName)
			throws AdminException
	{
		refreshServices();
		return daemonManager.get(hostName).getCountRunningServices();
	}

	public int getCountServices(String hostName)
			throws AdminException
	{
		refreshServices();
		return daemonManager.get(hostName).getCountServices();
	}

	private void refreshServices()
			throws AdminException
	{
		final long currentTime = System.currentTimeMillis();
		if (currentTime - Constants.ServicesRefreshTimeoutMillis > serviceRefreshTimeStamp)
		{
			serviceRefreshTimeStamp = currentTime;
			Map services = daemonManager.refreshServices(smeManager.getSmes());
			logger.debug("Refresh services: " + services.size() + " services found");
			serviceManager.updateServices(services);
		}
	}

	public void deployAdministrableService(File incomingZip, ServiceInfo serviceInfo)
			throws AdminException
	{
		final String id = serviceInfo.getId();
		if (serviceManager.contains(id))
			throw new AdminException("Couldn't add new service \"" + id + "\": service with that ID already contained in system.");
		if (smeManager.contains(id))
			throw new AdminException("Couldn't add new service \"" + id + "\": SME with that ID already contained in system.");

		try
		{
			serviceManager.deployAdministrableService(incomingZip, serviceInfo);
			daemonManager.addService(serviceInfo);
			smeManager.add(serviceInfo.getSme());
		}
		catch (AdminException e)
		{
			logger.error("Couldn't deploy administrable service \"" + id + "\"", e);
			serviceManager.rollbackDeploy(serviceInfo.getHost(), id);
			if (smeManager.contains(id))
				smeManager.remove(id);
		}
	}



	/* ***************************************** smes **************************************************/

	public List getSmeIds()
	{
		return smeManager.getSmeNames();
	}

	public SME addSme(String id, int priority, byte type, int typeOfNumber, int numberingPlan, int interfaceVersion, String systemType,
							String password, String addrRange, int smeN, boolean wantAlias, int timeout)
			throws AdminException
	{
		return smeManager.add(id, priority, type, typeOfNumber, numberingPlan, interfaceVersion, systemType, password,
									 addrRange, smeN, wantAlias, timeout);
	}

	SME getSme(String id) throws AdminException
	{
		return smeManager.get(id);
	}

	public void removeSme(String smeId)
			throws AdminException
	{
		if (serviceManager.contains(smeId))
			throw new AdminException("Couldn't remove sme \"" + smeId + "\" becouse it is service");

		smeManager.remove(smeId);
	}


	//? public
	public boolean containsSme(String id)
	{
		return smeManager.contains(id);
	}

	boolean isSmeUsed(String smeId) throws AdminException
	{
		return smeManager.isUsed(smeId);
	}

	public int getHostPort(String hostName) throws AdminException
	{
		return daemonManager.get(hostName).getPort();
	}

	public boolean isService(String smeId)
	{
		return serviceManager.contains(smeId);
	}

	public boolean isServiceAdministarble(String smeId)
	{
		return serviceManager.isServiceAdministarble(smeId);
	}

	public ServiceInfo getServiceInfo(String serviceId) throws AdminException
	{
		refreshServices();
		return serviceManager.getInfo(serviceId);
	}

	public Daemon getSmscDaemon()
	{
		return daemonManager.getSmscDaemon();
	}
}
