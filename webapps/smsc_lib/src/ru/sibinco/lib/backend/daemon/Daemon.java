package ru.sibinco.lib.backend.daemon;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.sibinco.lib.Constants;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.protocol.Proxy;
import ru.sibinco.lib.backend.protocol.Response;
import ru.sibinco.lib.backend.sme.SmeManager;
import ru.sibinco.lib.backend.util.SortedList;

import java.util.*;


/**
 * Created by igork Date: 03.06.2004 Time: 15:58:04
 */
public class Daemon extends Proxy
{
  public static final long REFRESH_TIMEOUT = 1000;

  private Map services = new HashMap();
  private String daemonServicesFolder;
  private long lastRefreshMillis = 0;

  public Daemon(final String host, final int port, final SmeManager smeManager, final String daemonServicesFolder)
      throws SibincoException
  {
    super(host, port);
    this.daemonServicesFolder = daemonServicesFolder;
    connect(host, port);
    refreshServices(smeManager);
  }

  public Map refreshServices(final SmeManager smeManager) throws SibincoException
  {
    final long now = System.currentTimeMillis();
    if (REFRESH_TIMEOUT < now - lastRefreshMillis) {
      final Response r = runCommand(new CommandListServices());
      if (Response.StatusOk != r.getStatus())
        throw new SibincoException("Couldn't list services, nested:" + r.getDataAsString());

      services.clear();

      final NodeList list = r.getData().getElementsByTagName("service");
      for (int i = 0; i < list.getLength(); i++) {
        final Element serviceElement = (Element) list.item(i);
        final ServiceInfo newInfo = new ServiceInfo(serviceElement, host, smeManager, daemonServicesFolder);
        services.put(newInfo.getId(), newInfo);
      }
      lastRefreshMillis = now;
    }
    return services;
  }

  public void startService(final String serviceId)
      throws SibincoException
  {
    requireService(serviceId);

    final Response r = runCommand(new CommandStartService(serviceId));
    if (Response.StatusOk != r.getStatus())
      throw new SibincoException("Couldn't start services \"" + serviceId + "\", nested:" + r.getDataAsString());
  }

  public ServiceInfo getServiceInfo(final String serviceId)
  {
    return (ServiceInfo) services.get(serviceId);
  }

  private void requireService(final String serviceId) throws SibincoException
  {
    if (!services.containsKey(serviceId))
      throw new SibincoException("Service \"" + serviceId + "\" not found on host \"" + host + "\"");
  }

  public void addService(final ServiceInfo serviceInfo)
      throws SibincoException
  {
    final String id = serviceInfo.getId();
    if (services.containsKey(id))
      throw new SibincoException("Couldn't add service \"" + id + "\" to host \"" + host + "\": service already contained in host");

    logger.debug("Add services \"" + id + "\" (" + serviceInfo.getHost() + ")");

    final Response r = runCommand(new CommandAddService(serviceInfo));
    if (Response.StatusOk != r.getStatus())
      throw new SibincoException("Couldn't add services \"" + id + '/' + id + "\" [" + serviceInfo.getArgs() + "], nested:" + r.getDataAsString());

    services.put(id, serviceInfo);
  }

  public void removeService(final String serviceId)
      throws SibincoException
  {
    requireService(serviceId);
    final Response r = runCommand(new CommandRemoveService(serviceId));
    if (Response.StatusOk != r.getStatus())
      throw new SibincoException("Couldn't remove services \"" + serviceId + "\", nested:" + r.getDataAsString());

    services.remove(serviceId);
  }

  public void shutdownService(final String serviceId)
      throws SibincoException
  {
    requireService(serviceId);
    final Response r = runCommand(new CommandShutdownService(serviceId));
    if (Response.StatusOk != r.getStatus()) {
      getServiceInfo(serviceId).setStatus(ServiceInfo.STATUS_UNKNOWN);
      throw new SibincoException("Couldn't shutdown services \"" + serviceId + "\", nested:" + r.getDataAsString());
    }
    getServiceInfo(serviceId).setStatus(ServiceInfo.STATUS_STOPPING);
  }

  public void killService(final String serviceId)
      throws SibincoException
  {
    requireService(serviceId);
    final Response r = runCommand(new CommandKillService(serviceId));
    if (Response.StatusOk != r.getStatus()) {
      getServiceInfo(serviceId).setStatus(ServiceInfo.STATUS_UNKNOWN);
      throw new SibincoException("Couldn't kill services \"" + serviceId + "\", nested:" + r.getDataAsString());
    }
    getServiceInfo(serviceId).setStatus(ServiceInfo.STATUS_STOPPED);
  }

  public List getServiceIds(final SmeManager smeManager) throws SibincoException
  {
    if (0 == services.size())
      refreshServices(smeManager);
    return new SortedList(services.keySet());
  }

  public void setServiceStartupParameters(final String serviceId, final String args)
      throws SibincoException
  {
    requireService(serviceId);
    final Response r = runCommand(new CommandSetServiceStartupParameters(serviceId, port, args));
    if (Response.StatusOk != r.getStatus())
      throw new SibincoException("Couldn't set services startup parameters \"" + serviceId + "\", nested:" + r.getDataAsString());
    final ServiceInfo serviceInfo = getServiceInfo(serviceId);
    serviceInfo.setArgs(args);
  }

  public boolean isContainsSmsc()
  {
    return isContainsService(Constants.SMSC_SME_ID);
  }

  public boolean isContainsService(final String serviceId)
  {
    return services.keySet().contains(serviceId);
  }

  public int getCountRunningServices()
  {
    int result = 0;
    for (Iterator i = services.values().iterator(); i.hasNext();) {
      final ServiceInfo info = (ServiceInfo) i.next();
      if (ServiceInfo.STATUS_RUNNING == info.getStatus() && !info.getId().equals(Constants.SMSC_SME_ID))
        result++;
    }
    return result;
  }

  public int getCountServices()
  {
    final Set serviceIds = new HashSet(services.keySet());
    serviceIds.remove(Constants.SMSC_SME_ID);
    return serviceIds.size();
  }

  public Map getServices()
  {
    return services;
  }

  public void removeAllServices() throws SibincoException
  {
    final Set serviceIds = new HashSet(services.keySet());
    for (Iterator i = serviceIds.iterator(); i.hasNext();) {
      final String serviceId = (String) i.next();
      removeService(serviceId);
    }
  }

  public String getDaemonServicesFolder()
  {
    return daemonServicesFolder;
  }
}
