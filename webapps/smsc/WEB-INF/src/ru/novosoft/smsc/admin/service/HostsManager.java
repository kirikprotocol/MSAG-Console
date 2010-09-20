package ru.novosoft.smsc.admin.service;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.daemon.Daemon;
import ru.novosoft.smsc.admin.daemon.DaemonManager;
import ru.novosoft.smsc.admin.resource_group.ResourceGroup;
import ru.novosoft.smsc.admin.route.SME;
import ru.novosoft.smsc.util.config.Config;

import java.io.File;
import java.io.IOException;
import java.util.List;
import java.util.Map;

/**
 * Created by igork Date: Jan 20, 2003 Time: 6:23:33 PM
 */
public interface  HostsManager {
    //only single
    public List getHostNames();

    public Daemon addHost(final String host, final int port, final String hostServicesFolder) throws AdminException;

    public Daemon removeHost(final String host) throws AdminException;

    public DaemonManager getDaemonManager();

    public void applyHosts() throws IOException, AdminException, Config.WrongParamTypeException;

    public int getHostPort(final String name) throws AdminException;

    public void deployAdministrableService(final File incomingZip, final ServiceInfo serviceInfo) throws AdminException;

    //only HA
    public void switchOver(final String serviceId) throws AdminException;

    /* **************************************** services ************************************************/
    public List getServiceIds() throws AdminException;

    public Map getServices(final String hostName) throws AdminException;

    public Service removeService(final String serviceId) throws AdminException;

    public void startService(final String serviceId) throws AdminException;

    public void killService(final String serviceId) throws AdminException;

    public void shutdownService(final String serviceId) throws AdminException;

    public int getCountRunningServices(final String hostName) throws AdminException;

    public int getCountServices(final String hostName) throws AdminException;

    public void refreshServices() throws AdminException;

    /* ***************************************** smes **************************************************/

    public List getSmeIds();

    public SME addSme(SME sme) throws AdminException;

    public void removeSme(final String smeId) throws AdminException;

    public boolean isService(final String smeId);

    public boolean isServiceAdministrable(final String smeId);

    public ServiceInfo getServiceInfo(final String serviceId) throws AdminException;

    public String getDaemonServicesFolder(final String resGroupName) throws AdminException;

    public byte getServiceStatus(final String serviceId) throws AdminException;
}
