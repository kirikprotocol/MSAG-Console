/*
 * Created by igork
 * Date: Feb 28, 2002
 * Time: 12:33:36 PM
 */
package ru.novosoft.smsc.admin.service;

import org.apache.log4j.Category;

import java.util.*;
import java.io.IOException;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.daemon.Daemon;
import ru.novosoft.smsc.admin.daemon.DaemonManager;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.config.ConfigManager;


public class ServiceManager
{
  static public class IsNotInitializedException extends Exception
  {
    IsNotInitializedException(String s)
    {
      super(s);
    }
  }


  protected static ServiceManager serviceManager = null;
  private static boolean isInitialized = false;

  public static ServiceManager getInstance()
          throws IsNotInitializedException
  {
    if (!isInitialized)
      throw new IsNotInitializedException("Service Manager is not initialized. Make ServiceManager.Init(...) call before ServiceManager.getInstance()");
    if (serviceManager == null)
      serviceManager = new ServiceManager();
    return serviceManager;
  }

  public static void init(ConfigManager cfgManager)
  {
    configManager = cfgManager;
    isInitialized = configManager != null;
  }


  private Map services = new HashMap();
  private DaemonManager daemonManager = new DaemonManager();
  protected static ConfigManager configManager = null;
  protected Category logger = Category.getInstance(this.getClass().getName());

  protected ServiceManager()
  {
    logger.debug("creating ServiceManager");
    Config config = configManager.getConfig();
    Set daemons = config.getSectionChildSectionNames("daemons");
    for (Iterator i = daemons.iterator(); i.hasNext();)
    {
      String encodedName = (String) i.next();
      String daemonName = StringEncoderDecoder.decode(encodedName.substring(encodedName.lastIndexOf('.')+1));
      try
      {
        addDaemonInternal(daemonName, config.getInt(encodedName + ".port"));
      }
      catch (AdminException e)
      {
        logger.error("Couldn't add daemon \"" + encodedName + "\"", e);
      }
      catch (Config.ParamNotFoundException e)
      {
        logger.debug("Misconfigured daemon \"" + encodedName + "\", parameter port missing", e);
      }
      catch (Config.WrongParamTypeException e)
      {
        logger.debug("Misconfigured daemon \"" + encodedName + "\", parameter port misformatted", e);
      }
    }
  }

  protected void addDaemonInternal(String host, int port)
  throws AdminException
  {
    Daemon d = daemonManager.addDaemon(host, port);
    Map newServices = d.listServices();
    for (Iterator i = newServices.keySet().iterator(); i.hasNext();)
    {
      ServiceInfo info = (ServiceInfo) newServices.get((String) i.next());
      putService(new Service(info));
    }
  }

  public synchronized void addDaemon(String host, int port)
          throws AdminException
  {
    addDaemonInternal(host, port);
    Config config = configManager.getConfig();
    config.setInt("daemons." + StringEncoderDecoder.encode(host) + ".port", port);
    try
    {
      configManager.save();
    }
    catch (Exception e)
    {
      logger.error("Couldn't save config", e);
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
    if (s.getInfo().getPid() != 0)
    {
      s.refreshComponents();
    }
  }

  public synchronized void killService(String serviceName)
          throws AdminException
  {
    Service s = getService(serviceName);
    Daemon d = getDaemon(s.getInfo().getHost());

    d.killService(serviceName);
    s.getInfo().setPid(0);
  }

  public synchronized void shutdownService(String name)
          throws AdminException
  {
    Service s = getService(name);
    Daemon d = getDaemon(s.getInfo().getHost());
    d.shutdownService(name);
  }

  public Object callServiceMethod(String hostName, String serviceName, String componentName,
                                  String methodName, String returnTypeName, Map args)
          throws AdminException
  {
    Service s = getService(serviceName);
    if (!s.getInfo().getHost().equals(hostName))
      throw new AdminException("Wrong host name (\"" + hostName + "\")");
    Component c = (Component) s.getInfo().getComponents().get(componentName);
    if (c == null)
      throw new AdminException("Wrong component name (\"" + componentName + "\")");
    Method m = (Method) c.getMethods().get(methodName);
    if (m == null)
      throw new AdminException("Wrong method name (\"" + methodName + "\")");
    Type t = Type.getInstance(returnTypeName);
    if (!m.getType().equals(t))
      throw new AdminException("Wrong method return type (\"" + returnTypeName + "\")");
    Map params = m.getParams();
    if (!params.keySet().equals(args.keySet()))
      throw new AdminException("Wrong arguments");
    Map arguments = new HashMap();
    for (Iterator i = params.values().iterator(); i.hasNext();)
    {
      Parameter p = (Parameter) i.next();
      if (args.get(p.getName()) == null)
        throw new AdminException("Parameter \"" + p.getName() + "\" not specified");
      switch (p.getType().getId())
      {
        case Type.StringType:
          {
            arguments.put(p.getName(), args.get(p.getName()));
            break;
          }
        case Type.IntType:
          {
            arguments.put(p.getName(), Integer.decode((String) args.get(p.getName())));
            break;
          }
        case Type.BooleanType:
          {
            arguments.put(p.getName(), Boolean.valueOf((String) args.get(p.getName())));
            break;
          }
        default:
          {
            throw new AdminException("Unknown parameter \"" + p.getName() + "\" type \"" + p.getType().getName() + "\"");
          }
      }
    }
    return s.call(c, m, t, arguments);
  }

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
    for (Iterator i = services.keySet().iterator(); i.hasNext();)
    {
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

  public synchronized void removeDaemon(String host)
          throws AdminException
  {
    daemonManager.removeDaemon(host);
    refreshServices();
    Config config = configManager.getConfig();
    config.removeParam("daemons." + StringEncoderDecoder.encode(host) + ".port");
    try
    {
      configManager.save();
    }
    catch (Exception e)
    {
      logger.error("Couldn't save config", e);
    }
  }

  /************************************ helpers ******************************/
  public synchronized void refreshServices()
          throws AdminException
  {
    services.clear();
    for (Iterator i = daemonManager.getHosts().iterator(); i.hasNext();)
    {
      Daemon d = daemonManager.getDaemon((String) i.next());
      Map infos = d.listServices();
      for (Iterator j = infos.values().iterator(); j.hasNext();)
        putService(new Service((ServiceInfo) j.next()));
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
    if (s.getInfo().getPid() != 0)
    {
      try
      {
        s.refreshComponents();
      }
      catch (AdminException e)
      {
        s.getInfo().setPid(0);
      }
    }
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
