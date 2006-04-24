/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.service;

import ru.sibinco.lib.backend.util.xml.Utils;
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.lib.backend.util.SortedList;
import ru.sibinco.scag.backend.routing.Route;
import ru.sibinco.scag.backend.status.StatMessage;
import ru.sibinco.scag.backend.status.StatusManager;

import javax.xml.parsers.ParserConfigurationException;
import java.util.*;
import java.io.IOException;
import java.io.File;
import java.io.PrintWriter;
import java.io.OutputStreamWriter;
import java.io.FileOutputStream;

import org.xml.sax.SAXException;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.apache.log4j.Logger;

/**
 * The <code>ServiceProvidersManager</code> class represents
 * <p><p/>
 * Date: 06.02.2006
 * Time: 16:27:01
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class ServiceProvidersManager {

    private Logger logger = Logger.getLogger(this.getClass());
    private final Map serviceProviders = Collections.synchronizedMap(new TreeMap());
    private long lastUsedServiceProviderId = -1;
    private long lastUsedServiceId = -1;
    private final String configFilename;
    private static final String PARAM_NAME_LAST_USED_SERVICE_PROVIDER_ID = "last used provider id";
    private static final String PARAM_NAME_LAST_USED_SERVICE_ID = "last used service id";


    public ServiceProvidersManager(final String configFilename) {
        this.configFilename = configFilename;
    }

    public synchronized void init() throws IOException, ParserConfigurationException, SAXException {
        serviceProviders.clear();
        if (configFilename != null) {
            final Document document = Utils.parse(configFilename);
            final NodeList paramConfs = document.getDocumentElement().getElementsByTagName("param");
            final NodeList serviceProvidersRecords = document.getDocumentElement().getElementsByTagName("provider");
            for (int i = 0; i < paramConfs.getLength(); i++) {
                final Element paramElem = (Element) paramConfs.item(i);
                final String name = paramElem.getAttribute("name");
                try {
                    if (PARAM_NAME_LAST_USED_SERVICE_PROVIDER_ID.equals(name)) {
                        lastUsedServiceProviderId = Integer.decode(Utils.getNodeText(paramConfs.item(i))).intValue();
                    }
                    if (PARAM_NAME_LAST_USED_SERVICE_ID.equals(name)) {
                        lastUsedServiceId = Integer.decode(Utils.getNodeText(paramConfs.item(i))).intValue();
                    }

                } catch (NumberFormatException e) {
                    logger.error("Int parameter \"" + name + "\" misformatted: " + lastUsedServiceProviderId + ", skipped", e);
                }
            }

            for (int i = 0; i < serviceProvidersRecords.getLength(); i++) {
                final Element element = (Element) serviceProvidersRecords.item(i);
                final String id = element.getAttribute("id");
                ServiceProvider serviceProvider = new ServiceProvider(Long.valueOf(id));
                Node node = serviceProvidersRecords.item(i);
                NodeList nodeList = node.getChildNodes();
                for (int j = 0; j < nodeList.getLength(); j++) {
                    Node provider = nodeList.item(j);
                    if (provider.getNodeName().equalsIgnoreCase("name")) {
                        serviceProvider.setName(Utils.getNodeText(provider));
                    }
                    if (provider.getNodeName().equalsIgnoreCase("description")) {
                        serviceProvider.setDescription(Utils.getNodeText(provider));
                    }
                }
                final NodeList services = element.getElementsByTagName("service");

                for (int j = 0; j < services.getLength(); j++) {
                    final Element serviceElem = (Element) services.item(j);
                    final String serviceId = serviceElem.getAttribute("id");
                    Service service = new Service(Long.valueOf(serviceId));

                    NodeList nodeListS = services.item(j).getChildNodes();

                    for (int f = 0; f < nodeListS.getLength(); f++) {
                        Node serviceNode = nodeListS.item(f);
                        if (serviceNode.getNodeName().equalsIgnoreCase("name")) {
                            service.setName(Utils.getNodeText(serviceNode));
                        }
                        if (serviceNode.getNodeName().equalsIgnoreCase("description")) {
                            service.setDescription(Utils.getNodeText(serviceNode));
                        }
                    }
                    serviceProvider.getServices().put(service.getId(), service);

                }
                serviceProviders.put(serviceProvider.getId(), serviceProvider);
            }
        }
    }

    public synchronized void store() throws IOException {
        storeServiceProviders();
    }

    private void storeServiceProviders() throws IOException {
        File configFile = new File(configFilename);
        File configNew = Functions.createNewFilenameForSave(configFile);

        PrintWriter out = new PrintWriter(new OutputStreamWriter(new FileOutputStream(configNew), Functions.getLocaleEncoding()));
        Functions.storeConfigHeader(out, "service_providers", "services.dtd", System.getProperty("file.encoding").equals("Cp1251") ? "ISO-8859-1" : System.getProperty("file.encoding"));
        out.print(getParamXmlText());
        for (Iterator i = new SortedList(serviceProviders.keySet()).iterator(); i.hasNext();) {
            Long id = (Long) i.next();
            ServiceProvider serviceProvider = (ServiceProvider) serviceProviders.get(id);
            out.print(getXmlText(serviceProvider));
        }
        Functions.storeConfigFooter(out, "service_providers");
        out.flush();
        out.close();
        Functions.renameNewSavedFileToOriginal(configNew, configFile);
    }

    private String getParamXmlText() {
        StringBuffer buffer = new StringBuffer();
        buffer.append("   <param name=\"" + PARAM_NAME_LAST_USED_SERVICE_PROVIDER_ID + "\" type=\"int\">").
                append(getLastUsedServiceProviderId()).append("</param>").append("\n");
        buffer.append("   <param name=\"" + PARAM_NAME_LAST_USED_SERVICE_ID + "\" type=\"int\">").
                append(getLastUsedServiceId()).append("</param>").append("\n");
        return buffer.toString();
    }

    private String getXmlText(ServiceProvider serviceProvider) {
        StringBuffer buffer = new StringBuffer();
        Map services = serviceProvider.getServices();

        buffer.append("   <provider id=\"").append(serviceProvider.getId()).append("\">").append("\n");
        buffer.append("       <name>").append(serviceProvider.getName()).append("</name>").append("\n");
        buffer.append("       <description>").append(serviceProvider.getDescription()).append("</description>").append("\n");
        buffer.append("       <services>").append("\n");
        if (services != null) {
            for (Iterator i = new SortedList(services.keySet()).iterator(); i.hasNext();) {
                Long id = (Long) i.next();
                Service service = (Service) services.get(id);
                buffer.append("        <service id=\"").append(service.getId()).append("\">").append("\n");
                buffer.append("           <name>").append(service.getName()).append("</name>").append("\n");
                buffer.append("           <description>").append(service.getDescription()).append("</description>").append("\n");
                buffer.append("        </service>").append("\n");
            }
        }
        buffer.append("       </services>").append("\n");
        buffer.append("   </provider>").append("\n");
        return buffer.toString();
    }

    public synchronized void updateServiceProvider(final String user, final long id, final String name, final String description) throws NullPointerException {
        final ServiceProvider provider = (ServiceProvider) serviceProviders.get(new Long(id));
        if (null == provider)
            throw new NullPointerException("Service Provider \"" + id + "\" not found.");
        provider.setName(name);
        if (description != null)
            provider.setDescription(description);
        StatMessage message = new StatMessage(user, "Service Provider", "Changed service provider: " + name + ".");
        StatusManager.getInstance().addStatMessages(message);
    }

    public synchronized long createServiceProvider(final String user, final String name, final String description) throws NullPointerException {
        final ServiceProvider serviceProvider = new ServiceProvider(new Long(++lastUsedServiceProviderId), name);
        serviceProvider.setDescription(description);
        serviceProviders.put(serviceProvider.getId(), serviceProvider);
        StatMessage message = new StatMessage(user, "Service Provider", "Added new service provider: " + name + ".");
        StatusManager.getInstance().addStatMessages(message);
        return getLastUsedServiceProviderId();
    }

    public synchronized long createService(final String user, final long serviceProviderId, final Service service) throws NullPointerException {
        final ServiceProvider serviceProvider = (ServiceProvider) serviceProviders.get(new Long(serviceProviderId));
        if (null == serviceProvider)
            throw new NullPointerException("Service Provider \"" + serviceProvider + "\" not found.");
        service.setId(new Long(++lastUsedServiceId));
        serviceProvider.getServices().put(service.getId(), service);
        serviceProviders.put(serviceProvider.getId(), serviceProvider);
        StatMessage message = new StatMessage(user, "Service", "Added new service: " + service.getName() + ".");
        StatusManager.getInstance().addStatMessages(message);
        return getLastUsedServiceId();
    }

    public synchronized long updateService(final String user, final long serviceProviderId, final Service service) throws NullPointerException {
        final ServiceProvider serviceProvider = (ServiceProvider) serviceProviders.get(new Long(serviceProviderId));
        if (null == serviceProvider)
            throw new NullPointerException("Service Provider \"" + serviceProvider + "\" not found.");
        serviceProvider.getServices().keySet().remove(service.getId());
        serviceProvider.getServices().put(service.getId(), service);
        serviceProviders.put(serviceProvider.getId(), serviceProvider);
        StatMessage message = new StatMessage(user, "Service", "Changed service: " + service.getName() + ".");
        StatusManager.getInstance().addStatMessages(message);
        return service.getId().longValue();
    }


    public synchronized Map getServiceProviders() {
        return serviceProviders;
    }

    public synchronized long getLastUsedServiceProviderId() {
        return lastUsedServiceProviderId;
    }

    public synchronized long getLastUsedServiceId() {
        return lastUsedServiceId;
    }

    public synchronized Service getServiceById(Long id) {
        Service service = null;
        for (Iterator i = new SortedList(serviceProviders.keySet()).iterator(); i.hasNext();) {
            Long serviceProviderId = (Long) i.next();
            ServiceProvider serviceProvider = (ServiceProvider) serviceProviders.get(serviceProviderId);
            if (serviceProvider.getServices().get(id) != null)
                return (Service) serviceProvider.getServices().get(id);
        }
        return service;
    }
    
    public synchronized ServiceProvider getServiceProviderByServiceId(Long id) {
        ServiceProvider service = null;
        for (Iterator i = new SortedList(serviceProviders.keySet()).iterator(); i.hasNext();) {
            Long serviceProviderId = (Long) i.next();
            ServiceProvider serviceProvider = (ServiceProvider) serviceProviders.get(serviceProviderId);
            if (serviceProvider.getServices().get(id) != null)
                return serviceProvider;
        }
        return service;
    }

    public synchronized Map getRoutesByServiceId(final Map routes, Long serviceId){
        Map result = Collections.synchronizedMap(new HashMap());
        for (Iterator i = new SortedList(routes.keySet()).iterator(); i.hasNext();) {
            String id = (String) i.next();
            Route route = (Route) routes.get(id);
            if (route.getService() != null) {
                if (route.getService().getId().equals(serviceId)) {
                    result.put(route.getId(), route);
                }
            }
        }
        return result;
    }
}
