package ru.novosoft.smsc.admin.service;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.daemon.Daemon;
import ru.novosoft.smsc.admin.resource_group.ResourceGroup;
import ru.novosoft.smsc.admin.resource_group.ResourceGroupManager;
import ru.novosoft.smsc.admin.route.SME;
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

/**
 * Created by igork Date: Jan 20, 2003 Time: 6:23:33 PM
 */
public class HostsManagerHAImpl implements HostsManager {
    private Category logger = Category.getInstance(this.getClass());

    private ResourceGroupManager resourceGroupManager = null;
    private ServiceManager serviceManager = null;
    private SmeManager smeManager = null;
    private RouteSubjectManager routeSubjectManager;
    private long serviceRefreshTimeStamp = 0;

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

    public synchronized List getHostNames() {
        //return resourceGroupManager.getResourceGroupsNames();
        return new SortedList();
    }

    /* **************************************** services ************************************************/
    public synchronized List getServiceIds() throws AdminException {
//    refreshServices();
        //return serviceManager.getServiceIds();*/
        return new SortedList();
    }

    public synchronized Map getServices(final String hostName) throws AdminException {
//    refreshServices();
        //return resourceGroupManager.get(hostName).getServices();
        return new HashMap();
    }

    public synchronized Service removeService(final String serviceId) throws AdminException {
/*	int useFlag = isSmeUsed(serviceId);
	if (useFlag != 0)
	{
		if (useFlag == 1) throw new AdminException("Service \"" + serviceId + "\" is used by routes");
		if (useFlag == 2) throw new AdminException("Service \"" + serviceId + "\" is used by subjects");
	}
    final ResourceGroup resGroup = resourceGroupManager.getServiceResourceGroup(serviceId);
    if (null == resGroup)
      throw new AdminException("Service \"" + serviceId + "\" host not found");

    resGroup.removeService(serviceId);
    final Service service = serviceManager.remove(serviceId);
    smeManager.remove(serviceId);

    return service;*/ return null;
    }

    public synchronized void startService(final String serviceId) throws AdminException {
        getService(serviceId).online();
/*    try {
      final ServiceInfo info = serviceManager.getInfo(serviceId);
      final ResourceGroup d = resourceGroupManager.get(info.getHost());
      d.startService(serviceId);
    } catch (AdminException e) {
      logger.error("Couldn't start service \"" + serviceId + "\"", e);
      throw e;
    }*/
    }

    public synchronized void killService(final String serviceId) throws AdminException {
/*    final ServiceInfo info = serviceManager.getInfo(serviceId);
    final ResourceGroup d = resourceGroupManager.get(info.getHost());

    d.killService(serviceId);*/
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
/*    refreshServices();
    return resourceGroupManager.get(hostName).getCountRunningServices();*/
        return 0;
    }

    public synchronized int getCountServices(final String hostName) throws AdminException {
//    refreshServices();
        return resourceGroupManager.getResourceGroupsCount();
    }

    public void refreshServices() throws AdminException {
        Map services = resourceGroupManager.refreshServices(smeManager);
        ServiceInfo smscInfo = serviceManager.getInfo(Constants.SMSC_SME_ID);
        smscInfo.setStatus(getServiceStatus(Constants.SMSC_SME_ID));
        services.put(Constants.SMSC_SME_ID, smscInfo);
//     logger.debug("Refresh services: " + services.size() + " services found");
        serviceManager.updateServices(services);
    }

    public synchronized void deployAdministrableService(final File incomingZip, final ServiceInfo serviceInfo) throws AdminException {
/*    final String id = serviceInfo.getId();
    if (serviceManager.contains(id))
      throw new AdminException("Couldn't add new service \"" + id + "\": service with that ID already contained in system.");
    if (smeManager.contains(id))
      throw new AdminException("Couldn't add new service \"" + id + "\": SME with that ID already contained in system.");
    String[] servicesFolderNames = resourceGroupManager.get(serviceInfo.getResGroup()).getServicesFolder();
    File[] serviceFolder = new File[servicesFolderNames.length];
    for (int i = 0; i < servicesFolderNames.length; i++)
     serviceFolder[i] = new File(servicesFolderNames[i], serviceInfo.getId());
    try
	{
      serviceManager.deployAdministrableService(incomingZip, serviceInfo, serviceFolder);
      resourceGroupManager.addService(serviceInfo);
      smeManager.add(serviceInfo.getSme());
    }
	catch (AdminException e)
	{
      logger.error("Couldn't deploy administrable service \"" + id + "\"", e);
      serviceManager.rollbackDeploy(id, serviceFolder);
      if (smeManager.contains(id)) smeManager.remove(id);
      throw e;
    }*/
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
            throw new AdminException("Couldn't remove sme \"" + smeId + "\" because it is service");
        }

        int useFlag = isSmeUsed(smeId);
        if (useFlag != 0) {
            if (useFlag == 1) throw new AdminException("Couldn't remove sme \"" + smeId + "\" because it is used by routes");
            if (useFlag == 2) throw new AdminException("Couldn't remove sme \"" + smeId + "\" because it is used by subjects");
        }

        smeManager.remove(smeId);
        resourceGroupManager.refreshResGroupList();
    }


    private int isSmeUsed(final String smeId) {
        return routeSubjectManager.isSmeUsed(smeId);
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
