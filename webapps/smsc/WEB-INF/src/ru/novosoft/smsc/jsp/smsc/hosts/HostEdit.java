/*
 * Created by igork
 * Date: 24.10.2002
 * Time: 13:54:16
 */
package ru.novosoft.smsc.jsp.smsc.hosts;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.SmscBean;

public class HostEdit extends SmscBean
{//todo
	private String hostName = null;
	private int port = -1;
	private String mbSave = null;
	private String mbCancel = null;

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

	public int process()
	{
		if (port == -1)
			try
			{
				port = hostsManager.getHostPort(hostName);
			}
			catch (AdminException e)
			{
				logger.error("Couldn't get port for host \"" + hostName + "\"", e);
				return error(SMSCErrors.error.hosts.daemonNotFound, hostName);
			}
		if (mbSave != null)
		{
			try
			{
				if (hostsManager.getHostPort(hostName) != port)
					return error(SMSCErrors.error.notYetImplemented);
			}
			catch (AdminException e)
			{
				logger.error("Couldn't get port for host \"" + hostName + "\"", e);
				return error(SMSCErrors.error.hosts.daemonNotFound, hostName);
			}
			appContext.getStatuses().setHostsChanged(true);
			return RESULT_DONE;
		}
		else if (mbCancel != null)
			return RESULT_DONE;
		else
			return RESULT_OK;
	}

	public void setMbSave(String mbSave)
	{
		this.mbSave = mbSave;
	}

	public void setMbCancel(String mbCancel)
	{
		this.mbCancel = mbCancel;
	}
}
