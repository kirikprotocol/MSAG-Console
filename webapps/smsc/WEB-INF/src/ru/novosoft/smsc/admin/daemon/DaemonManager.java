/*
 * Created by igork
 * Date: Mar 20, 2002
 * Time: 4:08:25 PM
 */
package ru.novosoft.smsc.admin.daemon;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.smsc_service.Smsc;

import java.util.HashMap;
import java.util.Map;
import java.util.Set;


public class DaemonManager
{
	private Map daemons = new HashMap();
	private Smsc smsc = null;

	public DaemonManager(Smsc smsc)
	{
		this.smsc = smsc;
	}

	public Daemon addDaemon(String host, int port)
			  throws AdminException
	{
		if (daemons.containsKey(host))
			throw new AdminException("Daemon already connected on host \"" + host + "\"");

		Daemon d = new Daemon(host, port, smsc);
		daemons.put(host, d);
		return d;
	}

	public void removeDaemon(String host)
			  throws AdminException
	{
		if (!daemons.containsKey(host))
			throw new AdminException("Daemon on host \"" + host + "\" not known");

		daemons.remove(host);
	}

	public Daemon getDaemon(String host)
	{
		return (Daemon) daemons.get(host);
	}

	/**
	 * @return hostnames
	 */
	public Set getHosts()
	{
		return daemons.keySet();
	}
}
