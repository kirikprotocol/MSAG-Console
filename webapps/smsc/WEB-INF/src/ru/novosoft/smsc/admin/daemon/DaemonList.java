package ru.novosoft.smsc.admin.daemon;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.SortedList;

import java.util.*;

/**
 * Created by igork
 * Date: Jan 20, 2003
 * Time: 8:11:11 PM
 */
public class DaemonList
{
	private Map daemons = new HashMap();

	public Daemon add(Daemon d)
			throws AdminException
	{
		if (daemons.containsKey(d.getHost()))
			throw new AdminException("Host \"" + d.getHost() + "\" already contained in list");
		daemons.put(d.getHost(), d);
		return d;
	}

	public Daemon remove(String hostName)
			throws AdminException
	{
		requireDaemon(hostName);
		return (Daemon) daemons.remove(hostName);
	}

	private void requireDaemon(String hostName)
			throws AdminException
	{
		if (!daemons.containsKey(hostName))
			throw new AdminException("Unknown host \"" + hostName + "\"");
	}

	public List getHostNames()
	{
		return new SortedList(daemons.keySet());
	}

	public Daemon get(String hostName)
			throws AdminException
	{
		requireDaemon(hostName);
		return (Daemon) daemons.get(hostName);
	}

	public boolean contains(String hostName)
	{
		return daemons.containsKey(hostName);
	}

	public Iterator iterator()
	{
		return daemons.values().iterator();
	}
}
