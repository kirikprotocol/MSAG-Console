/*
 * Created by igork
 * Date: Mar 20, 2002
 * Time: 5:20:22 PM
 */
package ru.novosoft.smsc.admin.service;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.smsc_service.SmeManager;
import ru.novosoft.smsc.admin.route.SME;

import java.util.HashMap;
import java.util.Map;


public class ServiceInfo
{
	public static final byte STATUS_RUNNING = 0;
	public static final byte STATUS_STARTING = 1;
	public static final byte STATUS_STOPPING = 2;
	public static final byte STATUS_STOPPED = 3;
	public static final byte STATUS_UNKNOWN = 4;

	protected String id = "";
	protected String host = "";
	protected int port = 0;
	protected String args = "";
	protected long pid = 0;
	protected Map components = new HashMap();
	protected SME sme = null;
	protected byte status = STATUS_STOPPED;


	public ServiceInfo(Element serviceElement, String serviceHost, SmeManager smeManager) throws AdminException
	{
		host = serviceHost;
		port = Integer.decode(serviceElement.getAttribute("port")).intValue();
		id = serviceElement.getAttribute("id");

		args = serviceElement.getAttribute("args");
		if (id.equals(""))
		{
			throw new AdminException("services name or services system id not specified in response");
		}
		if (id.equals(Constants.SMSC_SME_ID))
		{
			if (smeManager.contains(id))
				throw new AdminException("Couldn't add new SMSC - already presented");
			sme = new SME(id, 0, SME.SMPP, 0, 0, 0, "", "", "", 0, false, false, 0, "", false, SME.MODE_TRX);
		}
		else
		{
			sme = smeManager.get(id);
		}

		setStatusStr(serviceElement.getAttribute("status"));
		String pidStr = serviceElement.getAttribute("pid");
		this.pid = (pidStr != null && pidStr.length() > 0) ? Long.decode(pidStr).longValue() : 0;
	}

	private ServiceInfo(String id, String host, int port, String args, long pid, SME sme, byte status)
	{
		this.host = host;
		this.port = port;
		this.args = args;
		this.pid = pid;
		this.id = id;
		this.sme = sme;
		this.status = status;
	}

	public ServiceInfo(String id, String host, int port, String args, SME sme, byte status)
	{
		this(id, host, port, args, 0, sme, status);
	}


	public String getHost()
	{
		return host;
	}

	public int getPort()
	{
		return port;
	}

	public String getId()
	{
		return id;
	}

	public String getArgs()
	{
		return args;
	}

	public long getPid()
	{
		return pid;
	}

	public void setPid(long pid)
	{
		this.pid = pid;
		if (pid == 0 && (status == STATUS_RUNNING || status == STATUS_STOPPING))
			status = STATUS_STOPPED;
		if (pid != 0 && (status == STATUS_STARTING || status == STATUS_STOPPED))
			status = STATUS_RUNNING;
	}

	public Map getComponents()
	{
		return components;
	}

	public void setComponents(Element response)
	{
		components.clear();
		NodeList list = response.getElementsByTagName("component");
		for (int i = 0; i < list.getLength(); i++)
		{
			Element compElem = (Element) list.item(i);
			Component c = new Component(compElem);
			components.put(c.getName(), c);
		}
	}

	public SME getSme()
	{
		return sme;
	}

	public byte getStatus()
	{
		return status;
	}

	protected void setStatusStr(String statusStr)
	{
		if ("running".equalsIgnoreCase(statusStr))
		{
			this.status = STATUS_RUNNING;
		}
		else if ("starting".equalsIgnoreCase(statusStr))
		{
			this.status = STATUS_STARTING;
		}
		else if ("stopping".equalsIgnoreCase(statusStr))
		{
			this.status = STATUS_STOPPING;
		}
		else if ("stopped".equalsIgnoreCase(statusStr))
		{
			this.status = STATUS_STOPPED;
		}
		else
			this.status = STATUS_UNKNOWN;
	}

	public String getStatusStr()
	{
		switch (status)
		{
			case STATUS_RUNNING:
				return "running";
			case STATUS_STARTING:
				return "starting";
			case STATUS_STOPPING:
				return "stopping";
			case STATUS_STOPPED:
				return "stopped";
			case STATUS_UNKNOWN:
				return "unknown";
			default:
				return "unknown";
		}
	}

	public SME setSme(SME sme)
	{
		SME old_sme = this.sme;
		this.sme = sme;
		return old_sme;
	}

	public void setStatus(byte status)
	{
		this.status = status;
	}

	public void setPort(int port)
	{
		this.port = port;
	}

	public void setArgs(String args)
	{
		this.args = args;
	}
}
