/*
 * Created by igork
 * Date: 30.10.2002
 * Time: 2:32:11
 */
package ru.novosoft.smsc.jsp.smsc.hosts;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;

import java.util.List;

public class HostAdd extends HostsBean
{
	private String hostName = null;
	private int port = -1;
	private String mbSave = null;
	private String mbCancel = null;

	public int process(SMSCAppContext appContext, List errors)
	{
		int result = super.process(appContext, errors);
		if (result != RESULT_OK)
			return result;

		if (mbSave != null)
		{
			if (hostName == null || hostName.length() == 0)
				return error(SMSCErrors.error.hosts.hostNotSpecified);

			if (daemonManager.getHosts().contains(hostName))
				return error(SMSCErrors.error.hosts.hostAlreadyExist, hostName);

			if (port <= 0)
				return error(SMSCErrors.error.hosts.portNotSpecifiedOrIncorrect);

			try
			{
				serviceManager.addDaemon(hostName, port);
			}
			catch (AdminException e)
			{
				logger.error("Couldn't add host", e);
				return error(SMSCErrors.error.hosts.couldntAddHost, hostName, e);
			}
			appContext.getStatuses().setHostsChanged(true);
			return RESULT_DONE;
		}
		else if (mbCancel != null)
			return RESULT_DONE;

		if (hostName == null)
			hostName = "";
		if (port <= 0)
			port = 6680;

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

	public int getPort()
	{
		return port;
	}

	public void setPort(int port)
	{
		this.port = port;
	}

	public String getMbSave()
	{
		return mbSave;
	}

	public void setMbSave(String mbSave)
	{
		this.mbSave = mbSave;
	}

	public String getMbCancel()
	{
		return mbCancel;
	}

	public void setMbCancel(String mbCancel)
	{
		this.mbCancel = mbCancel;
	}
}
