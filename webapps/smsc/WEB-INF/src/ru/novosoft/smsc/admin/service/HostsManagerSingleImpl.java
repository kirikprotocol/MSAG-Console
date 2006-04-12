package ru.novosoft.smsc.admin.service;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.daemon.Daemon;
import ru.novosoft.smsc.admin.daemon.DaemonManagerImpl;
import ru.novosoft.smsc.admin.daemon.DaemonManager;
import ru.novosoft.smsc.admin.smsc_service.RouteSubjectManager;
import ru.novosoft.smsc.admin.smsc_service.SmeManager;

import java.util.Iterator;
import java.util.List;


/**
 * Created by igork Date: Jan 20, 2003 Time: 6:23:33 PM
 */
public class HostsManagerSingleImpl extends HostsManagerImpl {
    private DaemonManagerImpl daemonManager;

    public HostsManagerSingleImpl(final DaemonManager daemonManager, final ServiceManager serviceManager, final SmeManager smeManager,
                                  final RouteSubjectManager routeSubjectManager) {
        super(daemonManager, serviceManager, smeManager, routeSubjectManager);
        this.daemonManager = (DaemonManagerImpl) daemonManager;
    }

    public synchronized Daemon addHost(final String host, final int port, final String hostServicesFolder) throws AdminException {
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
        }
        catch (AdminException e) {
            daemonManager.remove(daemon.getHost());
            final AdminException exception = new AdminException("Couldn't add services from new host \"" + host + "\", host not added. Nested: " + e.getMessage());
            exception.initCause(e);
            throw exception;
        }
        return daemon;
    }

    public synchronized Daemon removeHost(final String host) throws AdminException {
        logger.debug("Remove host \"" + host + "\"");
        final Daemon daemon = daemonManager.get(host);
        if (daemon.isContainsSmsc()) {
            throw new AdminException("Couldn't remove host \"" + host + "\": host contains SMSC");
        }
        final List serviceIds = daemon.getServiceIds(smeManager);
        for (Iterator i = serviceIds.iterator(); i.hasNext();) {
            final String serviceId = (String) i.next();
            logger.debug("  " + serviceId);
        }
        for (Iterator i = serviceIds.iterator(); i.hasNext();) {
            final String serviceId = (String) i.next();
            if (isSmeUsed(serviceId) != 0) {
                throw new AdminException("SME \"" + serviceId + "\" is used");
            }
        }
        daemonManager.removeAllServicesFromHost(daemon.getHost());
        serviceManager.removeAll(serviceIds);
        smeManager.removeAllIfSme(serviceIds);
        return daemonManager.remove(daemon.getHost());
    }
}