/*
 * Created by igork
 * Date: 24.10.2002
 * Time: 21:56:26
 */
package ru.novosoft.smsc.jsp.smsc.hosts;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.daemon.Daemon;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.jsp.smsc.SmscBean;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

public class HostStoppingServices extends SmscBean
{
	protected String hostName = null;
	protected String serviceIds[] = new String[0];
	protected boolean waiting = true;

	protected Map services = null;
	protected Daemon daemon = null;

	public int process()
	{
		waiting = !waiting;

		daemon = daemonManager.getDaemon(hostName);
		if (daemon == null)
		{
			logger.warn("daemon \"" + hostName + "\" not found");
			return error("daemon \"" + hostName + "\" not found");
		}
		try
		{
			services = daemon.listServices();
		}
		catch (AdminException e)
		{
			logger.warn("Couldn't get services list from \"" + hostName + "\" daemon", e);
			services = new HashMap();
			return error("Couldn't get services list from \"" + hostName + "\" daemon");
		}

		if (serviceIds == null || serviceIds.length == 0)
			return RESULT_DONE;

		List notStopped = new LinkedList();
		for (int i = 0; i < serviceIds.length; i++)
		{
			String id = serviceIds[i];
			ServiceInfo info = (ServiceInfo) services.get(id);
			if (info == null)
				return error("Service \"" + id + "\" not found");

			if (info.getStatus() == ServiceInfo.STATUS_STOPPING)
				notStopped.add(id);
		}
		if (notStopped.size() == 0)
			return RESULT_DONE;

		serviceIds = (String[]) notStopped.toArray(new String[0]);
		return RESULT_OK;
	}

	public String getHostName()
	{
		return hostName;
	}

	public void setHostName(String hostName)
	{
		this.hostName = hostName;
	}

	public String[] getServiceIds()
	{
		return serviceIds;
	}

	public void setServiceIds(String[] serviceIds)
	{
		this.serviceIds = serviceIds;
	}

	public boolean isWaiting()
	{
		return waiting;
	}

	public void setWaiting(boolean waiting)
	{
		this.waiting = waiting;
	}
}
