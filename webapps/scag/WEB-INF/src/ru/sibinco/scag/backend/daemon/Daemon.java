/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.backend.daemon;

import ru.sibinco.scag.backend.endpoints.SmppManager;
import ru.sibinco.scag.backend.protocol.response.Response;
import ru.sibinco.scag.Constants;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.util.SortedList;

import java.util.Map;
import java.util.HashMap;
import java.util.List;
import java.util.Iterator;
import java.util.Set;
import java.util.HashSet;

import org.w3c.dom.NodeList;
import org.w3c.dom.Element;

/**
 * The <code>Daemon</code> class represents
 * <p><p/>
 * Date: 27.10.2005
 * Time: 13:10:12
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Daemon extends Proxy {

    public static final long REFRESH_TIMEOUT = 1000;

    private Map services = new HashMap();
    private String daemonServicesFolder;
    private long lastRefreshMillis = 0;

    public Daemon(final String host, final int port, final SmppManager smppManager,
                  final String daemonServicesFolder) throws SibincoException {
        super(host, port);
        this.daemonServicesFolder = daemonServicesFolder;
        try {
            connect(host, port);
            refreshServices(smppManager);
        } catch (SibincoException e) {
            logger.warn("Couldn't connect to \"" + host + ':' + port + "\", nested: " + e.getMessage());
        }
    }

    public Map refreshServices(final SmppManager smppManager) throws SibincoException {
        logger.debug( "Daemon.refreshServices() start" );
        if(getStatus() == STATUS_DISCONNECTED){
            logger.debug( "Daemon.refreshServices() STATUS_DISCONNECTED" );
            connect(getHost(), getPort());
        }
        final long now = System.currentTimeMillis();
//        System.out.println("Daemon.refreshServices() 'now - lastRefreshMillis'='" + (now - lastRefreshMillis)
//                + " '(getStatus() == STATUS_CONNECTED)='" + (getStatus() == STATUS_CONNECTED) + "'");
        if (REFRESH_TIMEOUT < now - lastRefreshMillis && getStatus() == STATUS_CONNECTED) {
            logger.debug( "Daemon.refreshServices() in IF" );
            final Response response = runCommand(new CommandListServices());
            if (Response.STATUS_OK != response.getStatus()) {
                throw new SibincoException("Couldn't list services, nested: " + response.getDataAsString());
            }
            services.clear();
//            System.out.println( "Daemon.refreshServices() response.getData()='" + response.getData() + "'" );
            final NodeList list = response.getData().getElementsByTagName("service"); //ToDo change on appropriate value
            for (int i = 0; i < list.getLength(); i++) {
                final Element serviceElement = (Element) list.item(i);
                final ServiceInfo serviceInfo = new ServiceInfo(serviceElement, host, smppManager, daemonServicesFolder);
                logger.debug("Daemon.refreshServices() serviceElement='" + serviceElement + "\n'getNodeName='" + serviceElement.getNodeName()
                        + "' serviceInfo.getStatus()='" + serviceInfo.getStatus() + "'");
                services.put(serviceInfo.getId(), serviceInfo);
            }
            lastRefreshMillis = now;

        }
        printServices();
        return services;
    }

    public void startService(final String serviceId) throws SibincoException {

        requireService(serviceId);

        final Response response = runCommand(new CommandStartService(serviceId));
        if (Response.STATUS_OK != response.getStatus())
            throw new SibincoException("Couldn't start services \"" + serviceId + "\", nested: " + response.getDataAsString());
    }

    public ServiceInfo getServiceInfo(final String serviceId) {
        logger.debug( "Daemon.getServiceInfo() start with serviceId='"  + serviceId + "'" );
//        System.out.println( "Daemon.getServiceInfo() printServices()" );
        printServices();
        return (ServiceInfo) services.get(serviceId);
    }

    private void printServices() {
//        System.out.println( "Daemon.printServices() start" );
        for( Iterator iter = services.keySet().iterator(); iter.hasNext(); ){
            Object key = iter.next();
            System.out.println( "Daemon.printServices() key='" + key + "' value='" + services.get(key)+ "'" );
        }
    }

    private void requireService(final String serviceId) throws SibincoException {
        if (!services.containsKey(serviceId))
            throw new SibincoException("Service \"" + serviceId + "\" not found on host \"" + host + "\"");
    }

    public void addService(final ServiceInfo serviceInfo) throws SibincoException {
        final String id = serviceInfo.getId();
        if (services.containsKey(id))
            throw new SibincoException("Couldn't add service \"" + id + "\" to host \"" + host + "\": service already contained in host");

        logger.debug("Add services \"" + id + "\" (" + serviceInfo.getHost() + ")");

        final Response response = runCommand(new CommandAddService(serviceInfo));
        if (Response.STATUS_OK != response.getStatus())
            throw new SibincoException("Couldn't add services \"" + id + "/" + id + "\" [" + serviceInfo.getArgs() + "], nested: " + response.getDataAsString());

        services.put(id, serviceInfo);
    }

    public void removeService(final String serviceId) throws SibincoException {
        requireService(serviceId);
        final Response response = runCommand(new CommandRemoveService(serviceId));
        if (Response.STATUS_OK != response.getStatus())
            throw new SibincoException("Couldn't remove services \"" + serviceId + "\", nested: " + response.getDataAsString());

        services.remove(serviceId);
    }

    public void shutdownService(final String serviceId) throws SibincoException {

        requireService(serviceId);
        final Response response = runCommand(new CommandShutdownService(serviceId));
        if (Response.STATUS_OK != response.getStatus()) {
            getServiceInfo(serviceId).setStatus(ServiceInfo.STATUS_UNKNOWN);
            throw new SibincoException("Couldn't shutdown services \"" + serviceId + "\", nested: " + response.getDataAsString());
        }
        getServiceInfo(serviceId).setStatus(ServiceInfo.STATUS_STOPPING);
    }

    public void killService(final String serviceId) throws SibincoException {

        requireService(serviceId);
        final Response response = runCommand(new CommandKillService(serviceId));
        if (Response.STATUS_OK != response.getStatus()) {
            getServiceInfo(serviceId).setStatus(ServiceInfo.STATUS_UNKNOWN);
            throw new SibincoException("Couldn't kill services \"" + serviceId + "\", nested: " + response.getDataAsString());
        }
        getServiceInfo(serviceId).setStatus(ServiceInfo.STATUS_STOPPED);
    }

    public List getServiceIds(final SmppManager smppManager) throws SibincoException {
        if (0 == services.size())
            refreshServices(smppManager);
        return new SortedList(services.keySet());
    }

    public boolean isContainsSmsc() {
        return isContainsServices(Constants.SCAG_SME_ID);
    }

    private boolean isContainsServices(String serviceId) {
        return services.keySet().contains(serviceId);
    }

    public int getCountRunningServices() {

        int result = 0;
        for (Iterator iterator = services.values().iterator(); iterator.hasNext();) {
            final ServiceInfo info = (ServiceInfo) iterator.next();
            if (ServiceInfo.STATUS_RUNNING == info.getStatus() && !info.getId().equals(Constants.SCAG_SME_ID))
                result++;
        }
        return result;
    }

    public int getCountServices() {
        final Set serviceIds = new HashSet(services.keySet());
        serviceIds.remove(Constants.SCAG_SME_ID);
        return serviceIds.size();
    }

    public void removeAllServices() throws SibincoException {
        final Set serviceIds = new HashSet(services.keySet());
        for (Iterator iterator = serviceIds.iterator(); iterator.hasNext();) {
            final String serviceId = (String) iterator.next();
            removeService(serviceId);
        }
    }

    public Map getServices() {
        return services;
    }

    public String getDaemonServicesFolder() {
        return daemonServicesFolder;
    }
}
