/*
 * Created by igork
 * Date: 29.10.2002
 * Time: 23:12:47
 */
package ru.novosoft.smsc.jsp.smsc.services;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.daemon.Daemon;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.jsp.SMSCErrors;

import java.util.*;

public class Statuses extends PageBean
{
	protected Map services = null;
	protected Daemon daemon = null;

	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

		services = new HashMap();
		Set hosts = serviceManager.getHostNames();
		for (Iterator i = hosts.iterator(); i.hasNext();)
		{
			String hostName = (String) i.next();
			daemon = daemonManager.getDaemon(hostName);
			if (daemon == null)
				error(SMSCErrors.error.hosts.daemonNotFound, hostName);
			else
			{
				try
				{
					services.putAll(daemon.listServices());
				}
				catch (Throwable e)
				{
					logger.error("Couldn't list services on host \"" + hostName + '"', e);
					error(SMSCErrors.warning.hosts.listFailed, hostName);
				}
			}
		}
		return errors.size() == 0 ? RESULT_OK : RESULT_ERROR;
	}

	public List getServiceIds()
	{
		return new LinkedList(services.keySet());
	}

	public byte getServiceStatus(String id)
	{
		return ((ServiceInfo) services.get(id)).getStatus();
	}
}
