/*
 * Created by igork
 * Date: 23.10.2002
 * Time: 21:16:20
 */
package ru.novosoft.smsc.jsp.smsc.hosts;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.SMSCAppContext;

import java.util.Collection;
import java.util.List;

public class Index extends HostsBean
{
	public static final int RESULT_ADD = PRIVATE_RESULT;
	public static final int RESULT_VIEW = PRIVATE_RESULT + 1;

	protected String mbAdd = null;
	protected String mbView = null;
	protected String hostName = null;


	public int process(SMSCAppContext appContext, List errors)
	{
		int result = super.process(appContext, errors);
		if (result != RESULT_OK)
			return result;

		if (mbAdd != null)
			return RESULT_ADD;
		else if (mbView != null)
			return RESULT_VIEW;
		else
			return RESULT_OK;
	}

	public Collection getHostNames()
	{
		return serviceManager.getHostNames();
	}

	public int getHostPort(String hostName)
	{
		return daemonManager.getDaemon(hostName).getPort();
	}

	public int getServicesTotal(String hostName)
	{
		try
		{
			return serviceManager.getCountServices(hostName);
		}
		catch (AdminException e)
		{
			logger.warn("host \"" + hostName + "\" not found", e);
		}
		return 0;
	}

	public int getServicesRunning(String hostName)
	{
		try
		{
			return serviceManager.getCountRunningServices(hostName);
		}
		catch (AdminException e)
		{
			logger.warn("host \"" + hostName + "\" not found", e);
		}
		return 0;
	}

	public String getMbAdd()
	{
		return mbAdd;
	}

	public void setMbAdd(String mbAdd)
	{
		this.mbAdd = mbAdd;
	}

	public String getMbView()
	{
		return mbView;
	}

	public void setMbView(String mbView)
	{
		this.mbView = mbView;
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
