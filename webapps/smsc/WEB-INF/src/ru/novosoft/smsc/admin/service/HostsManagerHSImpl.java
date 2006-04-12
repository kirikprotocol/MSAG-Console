package ru.novosoft.smsc.admin.service;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.daemon.Daemon;
import ru.novosoft.smsc.admin.daemon.DaemonManagerHSImpl;
import ru.novosoft.smsc.admin.route.SME;
import ru.novosoft.smsc.admin.smsc_service.RouteSubjectManager;
import ru.novosoft.smsc.admin.smsc_service.SmeManager;
import ru.novosoft.smsc.util.config.Config;

import java.io.File;
import java.io.IOException;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class HostsManagerHSImpl extends HostsManagerImpl {
    private DaemonManagerHSImpl daemonManager;

    public HostsManagerHSImpl(final DaemonManagerHSImpl daemonManager, final ServiceManager serviceManager, final SmeManager smeManager,
                              final RouteSubjectManager routeSubjectManager) {
        super(daemonManager, serviceManager, smeManager, routeSubjectManager);
        this.daemonManager = (DaemonManagerHSImpl) daemonManager;
    }

    public synchronized Map getServices(final String hostName) throws AdminException {
        if (!hostName.equals(daemonManager.getDaemon().getHost())) return new HashMap();
        return super.getServices(hostName);
    }

    public synchronized void switchOver(final String serviceId) throws AdminException {
        daemonManager.getDaemon().switchOver(serviceId);
    }

    public synchronized int getCountRunningServices(final String hostName) throws AdminException {
        if (!hostName.equals(daemonManager.getDaemon().getHost())) return 0;
        return super.getCountRunningServices(hostName);
    }

    public synchronized int getCountServices(final String hostName) throws AdminException {
        if (!hostName.equals(daemonManager.getDaemon().getHost())) return 0;
        return super.getCountServices(hostName);
    }

    public synchronized void applyHosts() throws IOException, AdminException, Config.WrongParamTypeException {
        daemonManager.store();
    }
}
