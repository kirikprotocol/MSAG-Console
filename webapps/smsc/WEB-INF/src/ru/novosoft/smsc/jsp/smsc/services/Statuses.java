/*
 * Created by igork
 * Date: 29.10.2002
 * Time: 23:12:47
 */
package ru.novosoft.smsc.jsp.smsc.services;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.SmeStatus;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.jsp.PageBean;

import java.util.List;
import java.util.LinkedList;

public class Statuses extends PageBean
{
	public List getServiceIds()
	{
		return appContext.getSmeManager().getSmeNames();
	}

	public byte getServiceStatus(String id)
	{
		try
		{
			if (hostsManager.isService(id))
				return hostsManager.getServiceInfo(id).getStatus();
			else
				return ServiceInfo.STATUS_UNKNOWN;
		}
		catch (AdminException e)
		{
			logger.error("Couldn't get service \"" + id + "\" status", e);
			return ServiceInfo.STATUS_UNKNOWN;
		}
	}

	public SmeStatus getSmeStatus(String id)
	{
		try
		{
			return appContext.getSmeManager().smeStatus(id);
		}
		catch (AdminException e)
		{
			logger.error("Couldn't get service \"" + id + "\" connection status, nested: " + e.getMessage());
			return null;
		}
	}
}
