/*
 * Created by igork
 * Date: Mar 20, 2002
 * Time: 4:08:25 PM
 */
package ru.novosoft.smsc.admin.daemon;

import java.util.*;

import ru.novosoft.smsc.admin.AdminException;

public class DaemonManager
{
  private Map daemons = new HashMap();

  public Daemon addDaemon(String host, int port)
    throws AdminException
  {
    if (daemons.containsKey(host))
      throw new AdminException("Daemon already connected on host \""+host+"\"");

    Daemon d = new Daemon(host, port);
    daemons.put(host, d);
    return d;
  }

  public void removeDaemon(String host)
  throws AdminException
  {
    if (!daemons.containsKey(host))
      throw new AdminException("Daemon on host \""+host+"\" not known");

    daemons.remove(host);
  }

  public Daemon getDaemon(String host)
  {
    return (Daemon)daemons.get(host);
  }

  public Set getHosts()
  {
    return daemons.keySet();
  }
}
