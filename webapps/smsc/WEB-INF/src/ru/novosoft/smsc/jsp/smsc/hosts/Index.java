/*
 * Created by igork
 * Date: 23.10.2002
 * Time: 21:16:20
 */
package ru.novosoft.smsc.jsp.smsc.hosts;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.SmscBean;

import java.util.*;

public class Index extends SmscBean
{
	public static final int RESULT_ADD = PRIVATE_RESULT;
	public static final int RESULT_VIEW = PRIVATE_RESULT + 1;

	protected String mbAdd = null;
	protected String mbView = null;
	protected String mbDelete = null;
	protected String hostName = null;
	protected String[] hostIds = new String[0];

	public int process(SMSCAppContext appContext, List errors)
	{
		int result = super.process(appContext, errors);
		if (result != RESULT_OK)
			return result;

		if (mbAdd != null)
			return RESULT_ADD;
		else if (mbView != null)
			return RESULT_VIEW;
		else if (mbDelete != null)
			return deleteHost();
		else
			return RESULT_OK;
	}

	protected int deleteHost()
	{
		if (hostIds == null || hostIds.length == 0)
			return error(SMSCErrors.error.hosts.hostsNotSpecified);

		List notRemovedIds = new LinkedList();

		for (int i = 0; i < hostIds.length; i++)
		{
			String id = hostIds[i];

			try
			{
				serviceManager.removeDaemon(id);
				appContext.getStatuses().setHostsChanged(true);
			}
			catch (AdminException e)
			{
				notRemovedIds.add(id);
				error(SMSCErrors.error.hosts.couldntRemoveHost, hostName);
			}
		}

		hostIds = (String[]) notRemovedIds.toArray(new String[0]);

		if (errors.size() == 0)
			return RESULT_OK;
		else
			return RESULT_ERROR;
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
			return serviceManager.getCountServices(hostName)
					- (hostName.equals(smsc.getInfo().getHost()) ? 1 : 0);
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
			return serviceManager.getCountRunningServices(hostName)
					- ((hostName.equals(smsc.getInfo().getHost()) && serviceManager.getServiceInfo(Constants.SMSC_SME_ID).getStatus() == ServiceInfo.STATUS_RUNNING) ? 1 : 0);
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

	public String getMbDelete()
	{
		return mbDelete;
	}

	public void setMbDelete(String mbDelete)
	{
		this.mbDelete = mbDelete;
	}

	public String[] getHostIds()
	{
		return hostIds;
	}

	public void setHostIds(String[] hostIds)
	{
		this.hostIds = hostIds;
	}
}
