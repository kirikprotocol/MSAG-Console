package ru.novosoft.smsc.admin.service;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.daemon.Daemon;
import ru.novosoft.smsc.admin.daemon.DaemonManager;
import ru.novosoft.smsc.admin.resource_group.ResourceGroup;
import ru.novosoft.smsc.admin.resource_group.ResourceGroupManager;
import ru.novosoft.smsc.admin.route.SME;
import ru.novosoft.smsc.admin.route.Route;
import ru.novosoft.smsc.admin.route.Subject;
import ru.novosoft.smsc.admin.smsc_service.RouteSubjectManager;
import ru.novosoft.smsc.admin.smsc_service.SmeManager;
import ru.novosoft.smsc.admin.smsc_service.SmscList;
import ru.novosoft.smsc.util.SortedList;
import ru.novosoft.smsc.util.WebAppFolders;
import ru.novosoft.smsc.util.config.Config;

import java.io.File;
import java.io.IOException;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Iterator;

public class HostsManagerHAImpl implements HostsManager {
    private Category logger = Category.getInstance(this.getClass());

    private ResourceGroupManager resourceGroupManager = null;
    private ServiceManager serviceManager = null;
    private SmeManager smeManager = null;
    private RouteSubjectManager routeSubjectManager;

    public HostsManagerHAImpl(final ResourceGroupManager resourceGroupManager, final ServiceManager serviceManager, final SmeManager smeManager,
                              final RouteSubjectManager routeSubjectManager) {
        this.resourceGroupManager = resourceGroupManager;
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

  public DaemonManager getDaemonManager() {
    return null;
  }

  public synchronized List getHostNames() {
        return new SortedList();
    }

    /* **************************************** services ************************************************/
    public synchronized List getServiceIds() throws AdminException {
        return new SortedList();
    }

    public synchronized Map getServices(final String hostName) throws AdminException {
        return new HashMap();
    }

    public synchronized Service removeService(final String serviceId) throws AdminException {
        return null;
    }

    public synchronized void startService(final String serviceId) throws AdminException {
        getService(serviceId).online();
    }

    public synchronized void killService(final String serviceId) throws AdminException {
    }

    public synchronized void shutdownService(final String serviceId) throws AdminException {
        getService(serviceId).offline();
    }

    public synchronized void switchOver(final String serviceId) throws AdminException {
        ResourceGroup rg = getService(serviceId);
        byte status = getServiceInfo(serviceId).getStatus();
        switch (status) {
            case ServiceInfo.STATUS_ONLINE1:
                rg.switchOver(SmscList.getNodeFromId(ServiceInfo.STATUS_ONLINE2));
                break;
            case ServiceInfo.STATUS_ONLINE2:
                rg.switchOver(SmscList.getNodeFromId(ServiceInfo.STATUS_ONLINE1));
                break;
            default:
                throw new AdminException("service " + serviceId + " is not online");
        }
    }

    public synchronized int getCountRunningServices(final String hostName) throws AdminException {
        return 0;
    }

    public synchronized int getCountServices(final String hostName) throws AdminException {
        return resourceGroupManager.getResourceGroupsCount();
    }

    public void refreshServices() throws AdminException {
        Map services = resourceGroupManager.refreshServices(smeManager);
        ServiceInfo smscInfo = serviceManager.getInfo(Constants.SMSC_SME_ID);
        smscInfo.setStatus(getServiceStatus(Constants.SMSC_SME_ID));
        services.put(Constants.SMSC_SME_ID, smscInfo);
        serviceManager.updateServices(services);
    }

    public synchronized void deployAdministrableService(final File incomingZip, final ServiceInfo serviceInfo) throws AdminException {
    }

    /* ***************************************** smes **************************************************/

    public synchronized List getSmeIds() {
        return smeManager.getSmeNames();
    }

    public synchronized SME addSme(final String id, final int priority, final byte type, final int typeOfNumber, final int numberingPlan,
                                   final int interfaceVersion, final String systemType, final String password, final String addrRange, final int smeN,
                                   final boolean wantAlias, final boolean forceDC, final int timeout, final String receiptSchemeName, final boolean disabled,
                                   final byte mode, final int proclimit, final int schedlimit, int accessMask) throws AdminException {
        return smeManager.add(id, priority, type, typeOfNumber, numberingPlan, interfaceVersion, systemType, password, addrRange, smeN, wantAlias, forceDC,
                timeout, receiptSchemeName, disabled, mode, proclimit, schedlimit, accessMask);
    }

    public synchronized void removeSme(final String smeId) throws AdminException {
        if (serviceManager.contains(smeId)) {
            throw new AdminException("Sme \"" + smeId + "\"  is service");
        }

        final List routesUsingSme = getRoutesUsingSme(smeId);
        if (!routesUsingSme.isEmpty())
          throw new AdminException("Sme \"" + smeId + "\"  used by routes: " + printRoutesList(routesUsingSme));

        final List subjectsUsingSme = getSubjectsUsingSme(smeId);
        if (!subjectsUsingSme.isEmpty())
          throw new AdminException("Sme \"" + smeId + "\"  used by subjects: " + printSubjectsList(subjectsUsingSme));

        smeManager.remove(smeId);
        resourceGroupManager.refreshResGroupList();
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

    public synchronized int getHostPort(final String name) throws AdminException {
        return 0;
    }

    public synchronized boolean isService(final String smeId) {
        return resourceGroupManager.contains(smeId);
    }

    public synchronized boolean isServiceAdministrable(final String smeId) {
        return serviceManager.isServiceAdministrable(smeId);
    }

    public synchronized ServiceInfo getServiceInfo(final String serviceId) throws AdminException {
        refreshServices();
        return serviceManager.getInfo(serviceId);
    }

    public synchronized ResourceGroup getService(final String smeId) throws AdminException {
        return resourceGroupManager.get(smeId);
    }

    public synchronized void applyHosts() throws IOException, AdminException, Config.WrongParamTypeException {
        resourceGroupManager.save();
    }

    public synchronized String getDaemonServicesFolder(final String resGroupName) throws AdminException {
        return WebAppFolders.getHAServicesFolder().getName();
    }

    public synchronized String[] getServiceNodes(final String serviceId) throws AdminException {
        return resourceGroupManager.get(serviceId).listNodes();
    }

    public synchronized byte getServiceStatus(final String serviceId) throws AdminException {
        byte result = ServiceInfo.STATUS_OFFLINE;
        ResourceGroup rg = getService(serviceId);
        result = rg.getOnlineStatus();
        return result;
    }
}
