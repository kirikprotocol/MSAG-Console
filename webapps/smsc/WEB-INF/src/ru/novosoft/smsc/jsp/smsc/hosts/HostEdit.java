/*
 * Created by igork
 * Date: 24.10.2002
 * Time: 13:54:16
 */
package ru.novosoft.smsc.jsp.smsc.hosts;

import ru.novosoft.smsc.admin.daemon.Daemon;
import ru.novosoft.smsc.jsp.smsc.SmscBean;

public class HostEdit extends SmscBean
{
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
		Daemon d = daemonManager.getDaemon(hostName);
		if (d == null)
			return error("Unknown host \"" + hostName + '"');
		if (port == -1)
			port = d.getPort();
		if (mbSave != null)
		{
			if (d.getPort() != port)
				return error("Not yet implemented");
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
