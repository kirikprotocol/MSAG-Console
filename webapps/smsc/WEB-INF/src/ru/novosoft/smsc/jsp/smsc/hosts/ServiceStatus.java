/*
 * Created by igork
 * Date: 29.10.2002
 * Time: 23:12:47
 */
package ru.novosoft.smsc.jsp.smsc.hosts;

import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.daemon.Daemon;

import java.util.*;

public class ServiceStatus extends HostsBean
{
	protected String hostName = null;

	protected Map services = null;
	protected Daemon daemon = null;

	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

		daemon = daemonManager.getDaemon(hostName);
		if (daemon == null)
			return error(SMSCErrors.error.service.hosts.daemonNotFound, hostName);

		try
		{
			services = daemon.listServices();
		}
		catch (AdminException e)
		{
			services = new HashMap();
			return error(SMSCErrors.warning.service.hosts.listFailed, hostName);
		}

		return RESULT_OK;
	}

	public Set getServiceIds()
	{
		return services.keySet();
	}

	public byte getServiceStatus(String id)
	{
		return ((ServiceInfo) services.get(id)).getStatus();
	}

	public String getHostName()
	{
		return hostName;
	}

	public void setHostName(String hostName)
	{
		this.hostName = hostName;
	}
}
