/*
 * Created by igork
 * Date: 29.10.2002
 * Time: 23:12:47
 */
package ru.novosoft.smsc.jsp.smsc.services;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.jsp.PageBean;

import java.util.List;
import java.util.LinkedList;

public class Statuses extends PageBean
{
	public List getServiceIds()
	{
		try
		{
			return hostsManager.getServiceIds();
		}
		catch (AdminException e)
		{
			logger.debug("couldn't get service ids", e);
			return new LinkedList();
		}
	}

	public byte getServiceStatus(String id)
	{
		try
		{
			return hostsManager.getServiceInfo(id).getStatus();
		}
		catch (AdminException e)
		{
			logger.error("Couldn't get service \"" + id + "\" status", e);
			return ServiceInfo.STATUS_UNKNOWN;
		}
	}
}
