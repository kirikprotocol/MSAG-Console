package ru.novosoft.smsc.admin.service;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.daemon.Daemon;
import ru.novosoft.smsc.admin.daemon.DaemonManager;
import ru.novosoft.smsc.admin.route.SME;
import ru.novosoft.smsc.admin.route.Route;
import ru.novosoft.smsc.admin.route.Subject;
import ru.novosoft.smsc.admin.smsc_service.RouteSubjectManager;
import ru.novosoft.smsc.admin.smsc_service.SmeManager;
import ru.novosoft.smsc.util.config.Config;

import java.io.File;
import java.io.IOException;
import java.util.List;
import java.util.Map;
import java.util.Iterator;

public abstract class HostsManagerImpl implements HostsManager {
    protected Category logger = Category.getInstance(this.getClass());
    protected DaemonManager daemonManager = null;
    protected ServiceManager serviceManager = null;
    protected SmeManager smeManager = null;
    protected RouteSubjectManager routeSubjectManager;
    protected long serviceRefreshTimeStamp = 0;

    public HostsManagerImpl(final DaemonManager daemonManager, final ServiceManager serviceManager, final SmeManager smeManager,
                            final RouteSubjectManager routeSubjectManager) {
        this.daemonManager = daemonManager;
        this.serviceManager = serviceManager;
        this.smeManager = smeManager;
        this.routeSubjectManager = routeSubjectManager;
        try {
            refreshServices();
        }
        catch (AdminException e) {
            logger.error("Couldn't refresh services, skipped", e);
        }
    }

    public synchronized Daemon addHost(final String host, final int port, final String hostServicesFolder) throws AdminException {
        return null;
    }

    public synchronized Daemon removeHost(final String host) throws AdminException {
        return null;
    }

    public synchronized List getHostNames() {
        return daemonManager.getHostNames();
    }

    public DaemonManager getDaemonManager() {
      return daemonManager;
    }

    public synchronized List getServiceIds() throws AdminException {
        refreshServices();
        return serviceManager.getServiceIds();
    }

    public synchronized Map getServices(final String hostName) throws AdminException {
        refreshServices();
        return daemonManager.get(hostName).getServices();
    }

    public synchronized Service removeService(final String serviceId) throws AdminException {
        final List routesUsingSme = getRoutesUsingSme(serviceId);
        if (!routesUsingSme.isEmpty())
          throw new AdminException("Service \"" + serviceId + "\" is used by routes: " + printRoutesList(routesUsingSme));

        final List subjectsUsingSme = getSubjectsUsingSme(serviceId);
        if (!subjectsUsingSme.isEmpty())
          throw new AdminException("Service \"" + serviceId + "\" is used by subjects: " + printSubjectsList(subjectsUsingSme));

        final Daemon daemon = daemonManager.getServiceDaemon(serviceId);
        if (null == daemon) {
            throw new AdminException("Service \"" + serviceId + "\" host not found");
        }

        daemon.removeService(serviceId);
        final Service service = serviceManager.remove(serviceId);
        smeManager.remove(serviceId);

        return service;
    }

    public synchronized void startService(final String serviceId) throws AdminException {
        try {
            final ServiceInfo info = serviceManager.getInfo(serviceId);
            final Daemon d = daemonManager.get(info.getHost());
            d.startService(serviceId);
        }
        catch (AdminException e) {
            logger.error("Couldn't start service \"" + serviceId + "\"", e);
            throw e;
        }
    }

    public synchronized void killService(final String serviceId) throws AdminException {
        final ServiceInfo info = serviceManager.getInfo(serviceId);
        final Daemon d = daemonManager.get(info.getHost());

        d.killService(serviceId);
    }

    public synchronized void shutdownService(final String serviceId) throws AdminException {
        daemonManager.get(serviceManager.getInfo(serviceId).getHost()).shutdownService(serviceId);
    }

    public synchronized void switchOver(final String serviceId) throws AdminException {
    }

    public synchronized int getCountRunningServices(final String hostName) throws AdminException {
        refreshServices();
        return daemonManager.get(hostName).getCountRunningServices();
    }

    public synchronized int getCountServices(final String hostName) throws AdminException {
        refreshServices();
        return daemonManager.get(hostName).getCountServices();
    }

    public void refreshServices() throws AdminException {
        final long currentTime = System.currentTimeMillis();
        if (currentTime - Constants.ServicesRefreshTimeoutMillis > serviceRefreshTimeStamp) {
            serviceRefreshTimeStamp = currentTime;
            final Map services = daemonManager.refreshServices(smeManager);
            logger.debug("Refresh services: " + services.size() + " services found");
            serviceManager.updateServices(services);
        }
    }

    public synchronized void deployAdministrableService(final File incomingZip, final ServiceInfo serviceInfo) throws AdminException {
        final String id = serviceInfo.getId();
        if (serviceManager.contains(id)) {
            throw new AdminException("Couldn't add new service \"" + id + "\": service with that ID already contained in system.");
        }
        if (smeManager.contains(id)) {
            throw new AdminException("Couldn't add new service \"" + id + "\": SME with that ID already contained in system.");
        }
        final File serviceFolder = new File(daemonManager.get(serviceInfo.getHost()).getDaemonServicesFolder(), serviceInfo.getId());
        try {
            serviceManager.deployAdministrableService(incomingZip, serviceInfo, serviceFolder);
            daemonManager.addService(serviceInfo);
            smeManager.add(serviceInfo.getSme());
        }
        catch (AdminException e) {
            logger.error("Couldn't deploy administrable service \"" + id + "\"", e);
            serviceManager.rollbackDeploy(serviceInfo.getHost(), id, serviceFolder);
            if (smeManager.contains(id)) {
                smeManager.remove(id);
            }
            throw e;
        }
    }

    /* ***************************************** smes **************************************************/

    public synchronized List getSmeIds() {
        return smeManager.getSmeNames();
    }

    public synchronized SME addSme(SME sme) throws AdminException {
        return smeManager.add(sme);
    }

    public synchronized void removeSme(final String smeId) throws AdminException {
        if (serviceManager.contains(smeId)) {
            throw new AdminException("Sme \"" + smeId + "\" is service");
        }

        final List routesUsingSme = getRoutesUsingSme(smeId);
        if (!routesUsingSme.isEmpty())
          throw new AdminException("Sme \"" + smeId + "\" is used by routes: " + printRoutesList(routesUsingSme));

        final List subjectsUsingSme = getSubjectsUsingSme(smeId);
        if (!subjectsUsingSme.isEmpty())
          throw new AdminException("Sme \"" + smeId + "\" is used by subjects: " + printSubjectsList(subjectsUsingSme));

        smeManager.remove(smeId);
    }

    protected int isSmeUsed(String smeId) {
      return routeSubjectManager.isSmeUsed(smeId);
    }

    private List getRoutesUsingSme(String smeId) {
      return routeSubjectManager.getRoutesUsingSme(smeId);
    }

    private List getSubjectsUsingSme(String smeId) {
      return routeSubjectManager.getSubjectsUsingSme(smeId);
    }

    private String printRoutesList(List routesList) {
      final StringBuffer result = new StringBuffer();
      for (Iterator iterator = routesList.iterator(); iterator.hasNext();)
        result.append(((Route)iterator.next()).getName()).append(iterator.hasNext() ? ", " : "");
      return result.toString();
    }

    private String printSubjectsList(List subjectsList) {
      final StringBuffer result = new StringBuffer();
      for (Iterator iterator = subjectsList.iterator(); iterator.hasNext();)
        result.append(((Subject)iterator.next()).getName()).append(iterator.hasNext() ? ", " : "");
      return result.toString();
    }

    public synchronized int getHostPort(final String hostName) throws AdminException {
        return daemonManager.get(hostName).getPort();
    }

    public synchronized boolean isService(final String smeId) {
        return serviceManager.contains(smeId);
    }

    public synchronized boolean isServiceAdministrable(final String smeId) {
        return serviceManager.isServiceAdministrable(smeId);
    }

    public synchronized byte getServiceStatus(final String serviceId) throws AdminException {
        return getServiceInfo(serviceId).getStatus();
    }

    public synchronized ServiceInfo getServiceInfo(final String serviceId) throws AdminException {
        refreshServices();
        return serviceManager.getInfo(serviceId);
    }

    public Service getService(final String smeId) throws AdminException {
        return serviceManager.get(smeId);
    }

    public synchronized void applyHosts() throws IOException, AdminException, Config.WrongParamTypeException {
        daemonManager.store();
    }

    public synchronized String getDaemonServicesFolder(final String hostName) throws AdminException {
        return daemonManager.get(hostName).getDaemonServicesFolder();
    }
}
