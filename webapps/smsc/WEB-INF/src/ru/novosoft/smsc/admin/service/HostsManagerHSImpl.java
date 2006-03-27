package ru.novosoft.smsc.admin.service;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.daemon.Daemon;
import ru.novosoft.smsc.admin.daemon.DaemonManagerHS;
import ru.novosoft.smsc.admin.route.SME;
import ru.novosoft.smsc.admin.smsc_service.RouteSubjectManager;
import ru.novosoft.smsc.admin.smsc_service.SmeManager;
import ru.novosoft.smsc.util.config.Config;

import java.io.File;
import java.io.IOException;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

public class HostsManagerHSImpl implements HostsManager {
    private Category logger = Category.getInstance(this.getClass());

    private DaemonManagerHS daemonManager = null;
    private ServiceManager serviceManager = null;
    private SmeManager smeManager = null;
    private RouteSubjectManager routeSubjectManager;
    private long serviceRefreshTimeStamp = 0;

    public HostsManagerHSImpl(final DaemonManagerHS daemonManager, final ServiceManager serviceManager, final SmeManager smeManager,
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

    /* **************************************** services ************************************************/
    public synchronized List getServiceIds() throws AdminException {
        refreshServices();
        return serviceManager.getServiceIds();
    }

    public synchronized Map getServices(final String hostName) throws AdminException {
        if (!hostName.equals(daemonManager.getDaemon().getHost())) return new HashMap();
        refreshServices();
        return daemonManager.getDaemon().getServices();
    }

    public synchronized Service removeService(final String serviceId) throws AdminException {
        int useFlag = isSmeUsed(serviceId);
        if (useFlag != 0) {
            if (useFlag == 1) throw new AdminException("Service \"" + serviceId + "\" is used by routes");
            if (useFlag == 2) throw new AdminException("Service \"" + serviceId + "\" is used by subjects");
        }
        final Daemon daemon = daemonManager.getDaemon();
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
            final Daemon d = daemonManager.getDaemon();
            d.startService(serviceId);
        }
        catch (AdminException e) {
            logger.error("Couldn't start service \"" + serviceId + "\"", e);
            throw e;
        }
    }

    public synchronized void killService(final String serviceId) throws AdminException {
        final Daemon d = daemonManager.getDaemon();
        d.killService(serviceId);
    }

    public synchronized void shutdownService(final String serviceId) throws AdminException {
        daemonManager.getDaemon().shutdownService(serviceId);
    }

    public synchronized void switchOver(final String serviceId) throws AdminException {
    }

    public synchronized int getCountRunningServices(final String hostName) throws AdminException {
        if (!hostName.equals(daemonManager.getDaemon().getHost())) return 0;
        refreshServices();
        return daemonManager.getDaemon().getCountRunningServices();
    }

    public synchronized int getCountServices(final String hostName) throws AdminException {
        if (!hostName.equals(daemonManager.getDaemon().getHost())) return 0;
        refreshServices();
        return daemonManager.getDaemon().getCountServices();
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
        final File serviceFolder = new File(daemonManager.getDaemon().getDaemonServicesFolder(), serviceInfo.getId());
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

    public synchronized SME addSme(final String id, final int priority, final byte type, final int typeOfNumber, final int numberingPlan,
                                   final int interfaceVersion, final String systemType, final String password, final String addrRange, final int smeN,
                                   final boolean wantAlias, final boolean forceDC, final int timeout, final String receiptSchemeName, final boolean disabled,
                                   final byte mode, final int proclimit, final int schedlimit, final int accessMask) throws AdminException {
        return smeManager.add(id, priority, type, typeOfNumber, numberingPlan, interfaceVersion, systemType, password, addrRange, smeN, wantAlias, forceDC,
                timeout, receiptSchemeName, disabled, mode, proclimit, schedlimit, accessMask);
    }

    public synchronized void removeSme(final String smeId) throws AdminException {
        if (serviceManager.contains(smeId)) {
            throw new AdminException("Couldn't remove sme \"" + smeId + "\" becouse it is service");
        }

        int useFlag = isSmeUsed(smeId);
        if (useFlag != 0) {
            if (useFlag == 1) throw new AdminException("Couldn't remove sme \"" + smeId + "\" because it is used by routes");
            if (useFlag == 2) throw new AdminException("Couldn't remove sme \"" + smeId + "\" because it is used by subjects");
        }

        smeManager.remove(smeId);
    }


    private int isSmeUsed(final String smeId) {
        return routeSubjectManager.isSmeUsed(smeId);
    }

    public synchronized int getHostPort(final String hostName) throws AdminException {
        return daemonManager.getDaemon().getPort();
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
        return daemonManager.getDaemon().getDaemonServicesFolder();
    }

}
