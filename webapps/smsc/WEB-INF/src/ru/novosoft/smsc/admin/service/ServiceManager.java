/*
 * Created by igork
 * Date: Feb 28, 2002
 * Time: 12:33:36 PM
 */
package ru.novosoft.smsc.admin.service;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.daemon.Daemon;
import ru.novosoft.smsc.admin.daemon.DaemonManager;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.config.ConfigManager;
import ru.novosoft.smsc.util.config.ConfigReader;

import java.io.*;
import java.util.*;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;


public class ServiceManager
{
  static public class IsNotInitializedException extends Exception
  {
    IsNotInitializedException(String s)
    {
      super(s);
    }
  }


  protected final static String SYSTEM_ID_PARAM_NAME = "system id";
  protected static ServiceManager serviceManager = null;
  private static boolean isInitialized = false;
  protected static File workFolder = new File("WEB-INF", "work");
  protected static File daemonsFolder = new File("WEB-INF", "daemons");

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
    for (Iterator i = daemons.iterator(); i.hasNext();) {
      String encodedName = (String) i.next();
      String daemonName = StringEncoderDecoder.decodeDot(encodedName.substring(encodedName.lastIndexOf('.') + 1));
      try {
        addDaemonInternal(daemonName, config.getInt(encodedName + ".port"));
      } catch (AdminException e) {
        logger.error("Couldn't add daemon \"" + encodedName + "\"", e);
      } catch (Config.ParamNotFoundException e) {
        logger.debug("Misconfigured daemon \"" + encodedName + "\", parameter port missing", e);
      } catch (Config.WrongParamTypeException e) {
        logger.debug("Misconfigured daemon \"" + encodedName + "\", parameter port misformatted", e);
      }
    }
  }

  protected void addDaemonInternal(String host, int port)
          throws AdminException
  {
    Daemon d = daemonManager.addDaemon(host, port);
    Map newServices = d.listServices();
    for (Iterator i = newServices.keySet().iterator(); i.hasNext();) {
      ServiceInfo info = (ServiceInfo) newServices.get((String) i.next());
      putService(new Service(info));
    }
  }

  public synchronized void addDaemon(String host, int port)
          throws AdminException
  {
    addDaemonInternal(host, port);
    Config config = configManager.getConfig();
    config.setInt("daemons." + StringEncoderDecoder.encodeDot(host) + ".port", port);
    try {
      configManager.save();
    } catch (Exception e) {
      logger.error("Couldn't save config", e);
    }
  }

  public synchronized Set getHostNames()
  {
    return daemonManager.getHosts();
  }

  protected File saveFileToTemp(InputStream in)
          throws IOException
  {
    File tmpFile = File.createTempFile("SMSC_SME_distrib_", ".zip.tmp");
    OutputStream out = new BufferedOutputStream(new FileOutputStream(tmpFile));

    byte buffer[] = new byte[2048];
    for (int readed = 0; (readed = in.read(buffer)) > -1;) {
      out.write(buffer, 0, readed);
    }

    in.close();
    out.close();
    return tmpFile;
  }

  public synchronized String receiveNewService(InputStream in)
          throws AdminException
  {
    try {
      File tmpFile = saveFileToTemp(in);

      ZipInputStream zin = new ZipInputStream(new BufferedInputStream(new FileInputStream(tmpFile)));
      ZipEntry entry = zin.getNextEntry();
      while (entry != null && !entry.equals("config.xml"))
        entry = zin.getNextEntry();
      if (entry == null)
        throw new AdminException("/config.xml not found in distributive");

      Config serviceConfig = ConfigReader.readConfig(zin);
      String systemId = serviceConfig.getString(SYSTEM_ID_PARAM_NAME);

      tmpFile.renameTo(new File(workFolder, systemId + ".zip"));
      logger.debug("Received new service \"" + systemId + "\"");
      return systemId;
    } catch (Config.ParamNotFoundException e) {
      throw new AdminException("Couldn't find parameter \"" + SYSTEM_ID_PARAM_NAME + "\" in distributive config, nested: " + e.getMessage());
    } catch (Config.WrongParamTypeException e) {
      throw new AdminException("Parameter \"" + SYSTEM_ID_PARAM_NAME + "\" in distributive config is not a string");
    } catch (Exception e) {
      throw new AdminException("Some exception occured, nested: " + e.getMessage());
    }
  }

  protected void unZipFileFromArchive(File folderUnpackTo, String name, ZipInputStream zin)
  throws IOException
  {
      File file = new File(folderUnpackTo, name);
      file.getParentFile().mkdirs();
      OutputStream out = new BufferedOutputStream(new FileOutputStream(file));
      for (int i = 0; (i = zin.read()) != -1; out.write(i));
      out.close();
  }

  protected void unZipArchive(File folderUnpackTo, InputStream in)
  throws IOException
  {
      ZipInputStream zin = new ZipInputStream(in);
      for (ZipEntry e = zin.getNextEntry(); e!= null; e = zin.getNextEntry())
      {
        unZipFileFromArchive(folderUnpackTo, e.getName(), zin);
      }
      zin.close();
      in.close();
  }

  public synchronized void addService(String service,
                                      String host,
                                      int port,
                                      String args,
                                      String systemId,
                                      String systemType,
                                      int typeOfNumber,
                                      int numberingPlan,
                                      int interfaceVersion,
                                      String rangeOfAddress,
                                      String smeNType)
          throws AdminException
  {
    logger.debug("Add service \"" + service + '/' + systemId + "\" (" + host + ':' + port + ")");
    try {
      unZipArchive(new File(new File(daemonsFolder, host), systemId),
                   new BufferedInputStream(new FileInputStream(new File(workFolder, systemId + ".zip"))));
    } catch (IOException e) {
      throw new AdminException("Couldn't unpack service, nested: " + e.getMessage());
    }

      ServiceInfo serviceInfo = new ServiceInfo(service,  systemId, host, port, args);

      Daemon d = getDaemon(serviceInfo.getHost());
      if (services.containsKey(serviceInfo.getId()))
        throw new AdminException("Service \"" + serviceInfo.getId() + "\" already present");

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
    if (s.getInfo().getPid() != 0) {
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
    for (Iterator i = params.values().iterator(); i.hasNext();) {
      Parameter p = (Parameter) i.next();
      if (args.get(p.getName()) == null)
        throw new AdminException("Parameter \"" + p.getName() + "\" not specified");
      switch (p.getType().getId()) {
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

  public synchronized Set getServiceIds()
  {
    return services.keySet();
  }

  public synchronized Set getServiceIds(String host)
          throws AdminException
  {
    if (!getHostNames().contains(host))
      throw new AdminException("Host \"" + host + "\" not connected");

    Set result = new HashSet();
    for (Iterator i = services.keySet().iterator(); i.hasNext();) {
      Service s = getService((String) i.next());
      if (s.getInfo().getHost().equals(host))
        result.add(s.getInfo().getId());
    }
    return result;
  }

  public synchronized ServiceInfo getServiceInfo(String servoceId)
          throws AdminException
  {
    Service s = getService(servoceId);
    return s.getInfo();
  }

  public synchronized void removeDaemon(String host)
          throws AdminException
  {
    daemonManager.removeDaemon(host);
    refreshServices();
    Config config = configManager.getConfig();
    config.removeParam("daemons." + StringEncoderDecoder.encode(host) + ".port");
    try {
      configManager.save();
    } catch (Exception e) {
      logger.error("Couldn't save config", e);
    }
  }

  public synchronized int getCountRunningServices(String hostName)
          throws AdminException
  {
    Set serviceIds = getServiceIds(hostName);
    int result = 0;
    for (Iterator i = serviceIds.iterator(); i.hasNext();) {
      String serviceId = (String) i.next();
      if (getServiceInfo(serviceId).getPid() != 0)
        result++;
    }
    return result;
  }

  public synchronized int getCountServices(String hostName)
          throws AdminException
  {
    return getServiceIds(hostName).size();
  }

  /************************************ helpers ******************************/
  public synchronized void refreshServices()
          throws AdminException
  {
    services.clear();
    for (Iterator i = daemonManager.getHosts().iterator(); i.hasNext();) {
      Daemon d = daemonManager.getDaemon((String) i.next());
      Map infos = d.listServices();
      for (Iterator j = infos.values().iterator(); j.hasNext();)
        putService(new Service((ServiceInfo) j.next()));
    }
  }

  public synchronized void refreshService(String serviceName)
          throws AdminException
  {
    Service s = getService(serviceName);
    Daemon d = daemonManager.getDaemon(s.getInfo().getHost());
    Map infos = d.listServices();
    for (Iterator j = infos.values().iterator(); j.hasNext();)
      replaceService(new Service((ServiceInfo) j.next()));
  }

  protected Service getService(String serviceId)
          throws AdminException
  {
    Service s = (Service) services.get(serviceId);
    if (s == null)
      throw new AdminException("Unknown service \"" + serviceId + '"');
    return s;
  }

  protected void putService(Service s)
          throws AdminException
  {
    if (services.containsKey(s.getInfo().getId()))
      throw new AdminException("Service \"" + s.getInfo().getId() + "\" already present");
    replaceService(s);
  }

  protected void replaceService(Service s)
          throws AdminException
  {
    services.put(s.getInfo().getId(), s);
    if (s.getInfo().getPid() != 0) {
      try {
        s.refreshComponents();
      } catch (AdminException e) {
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
