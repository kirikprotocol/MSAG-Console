package ru.novosoft.smsc.admin.service;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.daemon.Daemon;
import ru.novosoft.smsc.admin.daemon.DaemonManager;
import ru.novosoft.smsc.admin.route.SME;
import ru.novosoft.smsc.admin.smsc_service.RouteSubjectManager;
import ru.novosoft.smsc.admin.smsc_service.SmeManager;
import ru.novosoft.smsc.util.config.Config;

import java.io.File;
import java.io.IOException;
import java.util.*;


/**
 * Created by igork Date: Jan 20, 2003 Time: 6:23:33 PM
 */
public class HostsManager
{
  private Category logger = Category.getInstance(this.getClass());

  private DaemonManager daemonManager = null;
  private ServiceManager serviceManager = null;
  private SmeManager smeManager = null;
  private RouteSubjectManager routeSubjectManager;
  private long serviceRefreshTimeStamp = 0;

  public HostsManager(final DaemonManager daemonManager, final ServiceManager serviceManager, final SmeManager smeManager,
                      final RouteSubjectManager routeSubjectManager)
  {
    this.daemonManager = daemonManager;
    this.serviceManager = serviceManager;
    this.smeManager = smeManager;
    this.routeSubjectManager = routeSubjectManager;
    try {
      refreshServices();
    } catch (AdminException e) {
      logger.error("Couldn't refresh services, skipped", e);
    }
  }


  /* ******************************** daemon operations ***************************************/
  /**
   * Create and add daemon
   *
   * @param host
   * @param port
   * @return new added daemon
   * @throws AdminException
   */
  public synchronized Daemon addHost(final String host, final int port, final String hostServicesFolder) throws AdminException
  {
    final Daemon daemon = daemonManager.add(host, port, smeManager, hostServicesFolder);
    for (Iterator i = daemon.getServiceIds(smeManager).iterator(); i.hasNext();) {
      final String serviceId = (String) i.next();
      if (serviceManager.contains(serviceId)) {
        daemonManager.remove(daemon.getHost());
        throw new AdminException("Service \"" + serviceId + "\" from new host \"" + host + ":" + port + "\" already presented in system");
      }
    }
    try {
      serviceManager.addAllInfos(daemon.getServices());
    } catch (AdminException e) {
      daemonManager.remove(daemon.getHost());
      final AdminException exception = new AdminException("Couldn't add services from new host \"" + host + "\", host not added. Nested: " + e.getMessage());
      exception.initCause(e);
      throw exception;
    }
    return daemon;
  }

  public synchronized Daemon removeHost(final String host) throws AdminException
  {
    logger.debug("Remove host \"" + host + "\"");
    final Daemon daemon = daemonManager.get(host);
    if (daemon.isContainsSmsc())
      throw new AdminException("Couldn't remove host \"" + host + "\": host contains SMSC");
    final List serviceIds = daemon.getServiceIds(smeManager);
    for (Iterator i = serviceIds.iterator(); i.hasNext();) {
      final String serviceId = (String) i.next();
      logger.debug("  " + serviceId);
    }
    for (Iterator i = serviceIds.iterator(); i.hasNext();) {
      final String serviceId = (String) i.next();
      if (routeSubjectManager.isSmeUsed(serviceId))
        throw new AdminException("SME \"" + serviceId + "\" is used");
    }
    daemonManager.removeAllServicesFromHost(daemon.getHost());
    serviceManager.removeAll(serviceIds);
    smeManager.removeAllIfSme(serviceIds);
    return daemonManager.remove(daemon.getHost());
  }

  public synchronized List getHostNames()
  {
    return daemonManager.getHostNames();
  }

  /* **************************************** services ************************************************/
  public synchronized List getServiceIds() throws AdminException
  {
    refreshServices();
    return serviceManager.getServiceIds();
  }

  public synchronized Map getServices(final String hostName) throws AdminException
  {
    refreshServices();
    return daemonManager.get(hostName).getServices();
  }

  public synchronized Service removeService(final String serviceId) throws AdminException
  {
    if (routeSubjectManager.isSmeUsed(serviceId))
      throw new AdminException("Service \"" + serviceId + "\" is used by routes");
    final Daemon daemon = daemonManager.getServiceDaemon(serviceId);
    if (null == daemon)
      throw new AdminException("Service \"" + serviceId + "\" host not found");

    daemon.removeService(serviceId);
    final Service service = serviceManager.remove(serviceId);
    smeManager.remove(serviceId);

    return service;
  }

  public synchronized void startService(final String serviceId) throws AdminException
  {
    try {
      final ServiceInfo info = serviceManager.getInfo(serviceId);
      final Daemon d = daemonManager.get(info.getHost());
      d.startService(serviceId);
    } catch (AdminException e) {
      logger.error("Couldn't start service \"" + serviceId + "\"", e);
      throw e;
    }
  }

  public synchronized void killService(final String serviceId) throws AdminException
  {
    final ServiceInfo info = serviceManager.getInfo(serviceId);
    final Daemon d = daemonManager.get(info.getHost());

    d.killService(serviceId);
  }

  public synchronized void shutdownService(final String serviceId) throws AdminException
  {
    daemonManager.get(serviceManager.getInfo(serviceId).getHost()).shutdownService(serviceId);
  }

  public synchronized int getCountRunningServices(final String hostName) throws AdminException
  {
    refreshServices();
    return daemonManager.get(hostName).getCountRunningServices();
  }

  public synchronized int getCountServices(final String hostName) throws AdminException
  {
    refreshServices();
    return daemonManager.get(hostName).getCountServices();
  }

  public void refreshServices() throws AdminException
  {
    final long currentTime = System.currentTimeMillis();
    if (currentTime - Constants.ServicesRefreshTimeoutMillis > serviceRefreshTimeStamp) {
      serviceRefreshTimeStamp = currentTime;
      final Map services = daemonManager.refreshServices(smeManager);
      logger.debug("Refresh services: " + services.size() + " services found");
      serviceManager.updateServices(services);
    }
  }

  public synchronized void deployAdministrableService(final File incomingZip, final ServiceInfo serviceInfo) throws AdminException
  {
    final String id = serviceInfo.getId();
    if (serviceManager.contains(id))
      throw new AdminException("Couldn't add new service \"" + id + "\": service with that ID already contained in system.");
    if (smeManager.contains(id))
      throw new AdminException("Couldn't add new service \"" + id + "\": SME with that ID already contained in system.");
    final File serviceFolder = new File(daemonManager.get(serviceInfo.getHost()).getDaemonServicesFolder(), serviceInfo.getId());
    try {
      serviceManager.deployAdministrableService(incomingZip, serviceInfo, serviceFolder);
      daemonManager.addService(serviceInfo);
      smeManager.add(serviceInfo.getSme());
    } catch (AdminException e) {
      logger.error("Couldn't deploy administrable service \"" + id + "\"", e);
      serviceManager.rollbackDeploy(serviceInfo.getHost(), id, serviceFolder);
      if (smeManager.contains(id))
        smeManager.remove(id);
      throw e;
    }
  }





  /* ***************************************** smes **************************************************/

  public synchronized List getSmeIds()
  {
    return smeManager.getSmeNames();
  }

  public synchronized SME addSme(final String id, final int priority, final byte type, final int typeOfNumber, final int numberingPlan,
                                 final int interfaceVersion, final String systemType, final String password, final String addrRange, final int smeN,
                                 final boolean wantAlias, final boolean forceDC, final int timeout, final String receiptSchemeName, final boolean disabled,
                                 final byte mode, final int proclimit, final int schedlimit) throws AdminException
  {
    return smeManager.add(id, priority, type, typeOfNumber, numberingPlan, interfaceVersion, systemType, password, addrRange, smeN, wantAlias, forceDC,
                          timeout, receiptSchemeName, disabled, mode, proclimit, schedlimit);
  }

  public synchronized void removeSme(final String smeId) throws AdminException
  {
    if (serviceManager.contains(smeId))
      throw new AdminException("Couldn't remove sme \"" + smeId + "\" becouse it is service");

    if (isSmeUsed(smeId))
      throw new AdminException("Couldn't remove sme \"" + smeId + "\" becouse it is used by routes");

    smeManager.remove(smeId);
  }


  private boolean isSmeUsed(final String smeId)
  {
    return routeSubjectManager.isSmeUsed(smeId);
  }

  public synchronized int getHostPort(final String hostName) throws AdminException
  {
    return daemonManager.get(hostName).getPort();
  }

  public synchronized boolean isService(final String smeId)
  {
    return serviceManager.contains(smeId);
  }

  public synchronized boolean isServiceAdministarble(final String smeId)
  {
    return serviceManager.isServiceAdministarble(smeId);
  }

  public synchronized ServiceInfo getServiceInfo(final String serviceId) throws AdminException
  {
    refreshServices();
    return serviceManager.getInfo(serviceId);
  }

  public Service getService(final String smeId) throws AdminException
  {
    return serviceManager.get(smeId);
  }

  public synchronized void applyHosts() throws IOException, AdminException, Config.WrongParamTypeException
  {
    daemonManager.saveHosts();
  }

  public synchronized String getDaemonServicesFolder(final String hostName) throws AdminException
  {
    return daemonManager.get(hostName).getDaemonServicesFolder();
  }

/*
	public synchronized Daemon getSmscDaemon()
	{
		return daemonManager.getSmscDaemon();
	}
*/
}
