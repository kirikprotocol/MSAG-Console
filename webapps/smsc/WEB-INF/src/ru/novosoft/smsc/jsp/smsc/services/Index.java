/*
 * Created by igork
 * Date: 23.10.2002
 * Time: 19:49:14
 */
package ru.novosoft.smsc.jsp.smsc.services;

import ru.novosoft.smsc.admin.service.ServiceManager;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.PageBean;

import java.util.Collection;
import java.util.Vector;

public class Index extends PageBean
{
	public Collection getSmeIds()
	{
		return serviceManager.getSmeIds();
	}

	public boolean isService(String smeId)
	{
		return serviceManager.isService(smeId);
	}
}
