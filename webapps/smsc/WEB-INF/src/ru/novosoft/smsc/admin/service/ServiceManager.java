/*
 * Created by igork
 * Date: Feb 28, 2002
 * Time: 12:33:36 PM
 */
package ru.novosoft.smsc.admin.service;

import org.apache.log4j.Category;

import java.util.*;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.daemon.Daemon;
import ru.novosoft.smsc.admin.daemon.DaemonManager;

public class ServiceManager
{
  protected static ServiceManager manager = null;

  public static ServiceManager getInstance()
  {
    if (manager == null)
      manager = new ServiceManager();
    return manager;
  }


  private Map services = new HashMap();
  private DaemonManager daemonManager = new DaemonManager();
  protected Category logger = Category.getInstance(this.getClass().getName());

  protected ServiceManager()
  {
    logger.debug("creating ServiceManager");
  }

  public synchronized void addDaemon(String host, int port)
          throws AdminException
  {
    Daemon d = daemonManager.addDaemon(host, port);
    Map newServices = d.listServices();
    for (Iterator i = newServices.keySet().iterator(); i.hasNext();) {
      putService(new Service((ServiceInfo) newServices.get((String) i.next())));
    }
  }

  public synchronized Set getHosts()
  {
    return daemonManager.getHosts();
  }

  public synchronized void addService(ServiceInfo serviceInfo)
          throws AdminException
  {
    logger.debug("Add service \"" + serviceInfo.getName() + "\" (" + serviceInfo.getHost() + ':'
                 + serviceInfo.getPort() + ")");
    Daemon d = getDaemon(serviceInfo.getHost());
    if (services.containsKey(serviceInfo.getName()))
      throw new AdminException("Service \"" + serviceInfo.getName() + "\" already present");

    //logger.debug("checkpoint 1");
    d.addService(serviceInfo);
    //logger.debug("checkpoint 2");
    putService(new Service(serviceInfo));
    logger.debug("service added");
  }

  public synchronized void removeService(String serviceName)
          throws AdminException
  {
    Service s = getService(serviceName);
    Daemon d = getDaemon(s.getInfo().getHost());
    d.removeService(serviceName);
    services.remove(serviceName);
  }

  public synchronized void startService(String serviceName)
          throws AdminException
  {
    Service s = getService(serviceName);
    Daemon d = getDaemon(s.getInfo().getHost());
    s.getInfo().setPid(d.startService(serviceName));
  }

  public synchronized void killService(String serviceName)
          throws AdminException
  {
    Service s = getService(serviceName);
    Daemon d = getDaemon(s.getInfo().getHost());

    try {
      d.shutdownService(serviceName);
    } catch (AdminException e) {
      d.killService(serviceName);
    }
    s.getInfo().setPid(0);
  }

  public synchronized void shutdownService(String name)
          throws AdminException
  {
    Service s = getService(name);
    Daemon d = getDaemon(s.getInfo().getHost());
    d.shutdownService(name);
  }

/*  public synchronized Config getServiceConfig(String name)
          throws AdminException
  {
    Service s = getService(name);
    return s.getConfig();
  }

  public synchronized String getServiceLogs(String name, int startPos, int length)
          throws AdminException
  {
    Service s = getService(name);
    return s.getLogs(startPos, length);
  }

  public synchronized Map getServiceMonitoringData(String name)
          throws AdminException
  {
    Service s = getService(name);
    return s.getMonitoringData();
  }
*/
  public synchronized Set getServiceNames()
  {
    return services.keySet();
  }

  public synchronized Set getServiceNames(String host)
          throws AdminException
  {
    if (!getHosts().contains(host))
      throw new AdminException("Host \"" + host + "\" not connected");

    Set result = new HashSet();
    for (Iterator i = services.keySet().iterator(); i.hasNext();) {
      Service s = getService((String) i.next());
      if (s.getInfo().getHost().equals(host))
        result.add(s.getInfo().getName());
    }
    return result;
  }

  public synchronized ServiceInfo getServiceInfo(String servoceName)
          throws AdminException
  {
    Service s = getService(servoceName);
    return s.getInfo();
  }

  /************************************ helpers ******************************/
  public synchronized void refreshServices()
          throws AdminException
  {
    services.clear();
    for (Iterator i = daemonManager.getHosts().iterator(); i.hasNext();) {
      Daemon d = daemonManager.getDaemon((String) i.next());
      Map infos = d.listServices();
      for (Iterator j = infos.values().iterator(); j.hasNext();) {
        ServiceInfo info = (ServiceInfo) j.next();
        Service s = new Service(info);
        if (info.getPid() != 0)
        {
          try {
            s.refreshComponents();
          } catch (AdminException e) {
            s.getInfo().setPid(0);
          }
        }
        services.put(s.getInfo().getName(), s);
      }
    }
  }

  protected Service getService(String serviceName)
          throws AdminException
  {
    Service s = (Service) services.get(serviceName);
    if (s == null)
      throw new AdminException("Unknown service \"" + serviceName + '"');
    return s;
  }

  protected void putService(Service s)
          throws AdminException
  {
    if (services.containsKey(s.getInfo().getName()))
      throw new AdminException("Service \"" + s.getInfo().getName() + "\" already present");
    services.put(s.getInfo().getName(), s);
  }

  protected Daemon getDaemon(String hostName)
          throws AdminException
  {
    Daemon d = daemonManager.getDaemon(hostName);
    if (d == null)
      throw new AdminException("Host \"" + hostName + "\" not connected");
    return d;
  }

  protected void putDaemon(Daemon d)
          throws AdminException
  {
    if (services.containsKey(d.getHost()))
      throw new AdminException("Host \"" + d.getHost() + "\" already connected");
    services.put(d.getHost(), d);
  }
}
