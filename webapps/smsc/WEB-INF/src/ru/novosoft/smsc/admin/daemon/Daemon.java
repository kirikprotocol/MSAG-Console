/*
 * Created by igork
 * Date: Mar 20, 2002
 * Time: 4:05:54 PM
 */
package ru.novosoft.smsc.admin.daemon;

import org.apache.log4j.Category;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.protocol.*;
import ru.novosoft.smsc.admin.route.SMEList;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.utli.Proxy;
import ru.novosoft.smsc.util.SortedList;

import java.util.*;


public class Daemon extends Proxy
{
	private Category logger = Category.getInstance(this.getClass().getName());
	private Map services = new HashMap();

	public Daemon(String host, int port, SMEList smeList)
			throws AdminException
	{
		super(host, port);
		connect(host, port);
		refreshServices(smeList);
	}

	protected Map refreshServices(SMEList smeList) throws AdminException
	{
		Response r = runCommand(new CommandListServices());
		if (r.getStatus() != Response.StatusOk)
			throw new AdminException("Couldn't list services, nested:" + r.getDataAsString());

		services.clear();

		NodeList list = r.getData().getElementsByTagName("service");
		for (int i = 0; i < list.getLength(); i++)
		{
			final Element serviceElement = (Element) list.item(i);
			ServiceInfo newInfo = new ServiceInfo(serviceElement, host, smeList);
			services.put(newInfo.getId(), newInfo);
		}
		return services;
	}

	/**
	 * @return Process ID (PID) of new services
	 */
	public long startService(String serviceId)
			throws AdminException
	{
		requireService(serviceId);

		Response r = runCommand(new CommandStartService(serviceId));
		if (r.getStatus() != Response.StatusOk)
			throw new AdminException("Couldn't start services \"" + serviceId + "\", nested:" + r.getDataAsString());

		String pidStr = r.getDataAsString().trim();
		try
		{
			final long pid = Long.decode(pidStr).longValue();
			getServiceInfo(serviceId).setPid(pid);
			return pid;
		}
		catch (NumberFormatException e)
		{
			throw new AdminException("PID of new services misformatted (" + pidStr + "), nested:" + e.getMessage());
		}
	}

	private ServiceInfo getServiceInfo(String serviceId)
	{
		return (ServiceInfo) services.get(serviceId);
	}

	private void requireService(String serviceId) throws AdminException
	{
		if (!services.containsKey(serviceId))
			throw new AdminException("Service \"" + serviceId + "\" not found on host \"" + host + "\"");
	}

	public void addService(ServiceInfo serviceInfo)
			throws AdminException
	{
		final String id = serviceInfo.getId();
		if (services.containsKey(id))
			throw new AdminException("Couldn't add service \"" + id + "\" to host \"" + host + "\": service already contained in host");

		logger.debug("Add services \"" + id + "\" (" + serviceInfo.getHost() + ':' + serviceInfo.getPort() + ")");

		Response r = runCommand(new CommandAddService(serviceInfo));
		if (r.getStatus() != Response.StatusOk)
			throw new AdminException("Couldn't add services \"" + id + '/' + id + "\" [" + serviceInfo.getArgs() + "], nested:" + r.getDataAsString());

		services.put(id, serviceInfo);
	}

	public void removeService(String serviceId)
			throws AdminException
	{
		requireService(serviceId);
		Response r = runCommand(new CommandRemoveService(serviceId));
		if (r.getStatus() != Response.StatusOk)
			throw new AdminException("Couldn't remove services \"" + serviceId + "\", nested:" + r.getDataAsString());

		services.remove(serviceId);
	}

	public void shutdownService(String serviceId)
			throws AdminException
	{
		requireService(serviceId);
		Response r = runCommand(new CommandShutdownService(serviceId));
		if (r.getStatus() != Response.StatusOk)
		{
			getServiceInfo(serviceId).setStatus(ServiceInfo.STATUS_UNKNOWN);
			throw new AdminException("Couldn't shutdown services \"" + serviceId + "\", nested:" + r.getDataAsString());
		}
		getServiceInfo(serviceId).setStatus(ServiceInfo.STATUS_STOPPING);
	}

	public void killService(String serviceId)
			throws AdminException
	{
		requireService(serviceId);
		Response r = runCommand(new CommandKillService(serviceId));
		if (r.getStatus() != Response.StatusOk)
		{
			getServiceInfo(serviceId).setStatus(ServiceInfo.STATUS_UNKNOWN);
			throw new AdminException("Couldn't kill services \"" + serviceId + "\", nested:" + r.getDataAsString());
		}
		getServiceInfo(serviceId).setStatus(ServiceInfo.STATUS_STOPPED);
	}

	public List getServiceIds(SMEList smeList) throws AdminException
	{
		if (services.size() == 0)
			refreshServices(smeList);
		return new SortedList(services.keySet());
	}

	public void setServiceStartupParameters(String serviceId, int port, String args)
			throws AdminException
	{
		requireService(serviceId);
		Response r = runCommand(new CommandSetServiceStartupParameters(serviceId, port, args));
		if (r.getStatus() != Response.StatusOk)
			throw new AdminException("Couldn't set services startup parameters \"" + serviceId + "\", nested:" + r.getDataAsString());
		ServiceInfo serviceInfo = getServiceInfo(serviceId);
		serviceInfo.setPort(port);
		serviceInfo.setArgs(args);
	}

	public boolean isContainsSmsc()
	{
		return isContainsService(Constants.SMSC_SME_ID);
	}

	public boolean isContainsService(String serviceId)
	{
		return services.keySet().contains(serviceId);
	}

	public int getCountRunningServices()
	{
		int result = 0;
		for (Iterator i = services.values().iterator(); i.hasNext();)
		{
			ServiceInfo info = (ServiceInfo) i.next();
			if (info.getStatus() == ServiceInfo.STATUS_RUNNING && !info.getId().equals(Constants.SMSC_SME_ID))
				result++;
		}
		return result;
	}

	public int getCountServices()
	{
		Set serviceIds = new HashSet(services.keySet());
		serviceIds.remove(Constants.SMSC_SME_ID);
		return serviceIds.size();
	}

	public Map getServices()
	{
		return services;
	}

	public void removeAllServices() throws AdminException
	{
		Set serviceIds = new HashSet(services.keySet());
		for (Iterator i = serviceIds.iterator(); i.hasNext();)
		{
			String serviceId = (String) i.next();
			removeService(serviceId);
		}
	}
}
