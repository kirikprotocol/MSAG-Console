/*
 * Created by igork
 * Date: Mar 20, 2002
 * Time: 5:20:22 PM
 */
package ru.novosoft.smsc.admin.service;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.SME;
import ru.novosoft.smsc.admin.route.SMEList;

import java.util.HashMap;
import java.util.Map;


public class ServiceInfo
{
	protected String id = "";
	protected String host = "";
	protected int port = 0;
	protected String args = "";
	protected long pid = 0;
	protected Map components = new HashMap();
	protected SME sme = null;


	public ServiceInfo(Element serviceElement, String serviceHost, SMEList smes)
			  throws AdminException
	{
		host = serviceHost;
		port = Integer.decode(serviceElement.getAttribute("port")).intValue();
		id = serviceElement.getAttribute("id");

		args = serviceElement.getAttribute("args");
		if (/*name.equals("") ||*/ id.equals(""))
		{
			throw new AdminException("service name or service system id not specified in response");
		}
		sme = smes.get(id);
		if (sme == null)
			throw new AdminException("Unknown SME ID: \"" + id + '"');
	}

	private ServiceInfo(String id, String host, int port, String args, long pid, SME sme)
			  throws AdminException
	{
//    this.name = name;
		this.host = host;
		this.port = port;
		this.args = args;
		this.pid = pid;
		this.id = id;
		this.sme = sme;
/*
		if (smes != null)
		{
		  sme = smes.get(id);
		  if (sme == null)
			 throw new AdminException("Unknown SME ID: \"" + id + '"');
		}
*/
	}

	public ServiceInfo(String id, String host, int port, String args, SME sme)
			  throws AdminException
	{
		this(/*name, */id, host, port, args, 0, sme);
	}


/*  public String getName()
  {
    return name;
  }
*/
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

	public boolean isRunning()
	{
		return pid != 0;
	}

	public void setPid(long pid)
	{
		this.pid = pid;
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
}
