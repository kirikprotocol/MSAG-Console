/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.service;

import org.apache.log4j.Logger;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.StatusDisconnectedException;
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.lib.backend.util.SortedList;
import ru.sibinco.lib.backend.util.xml.Utils;
import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.Manager;
import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.backend.routing.Route;
import ru.sibinco.scag.backend.routing.http.HttpRoute;
import ru.sibinco.scag.backend.status.StatMessage;
import ru.sibinco.scag.backend.status.StatusManager;
import ru.sibinco.scag.beans.SCAGJspException;

import javax.xml.parsers.ParserConfigurationException;
import java.io.*;
import java.util.*;

/**
 * The <code>ServiceProvidersManager</code> class represents
 * <p><p/>
 * Date: 06.02.2006
 * Time: 16:27:01
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class ServiceProvidersManager extends Manager {

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

    public void store() throws SibincoException {
        try {
          storeServiceProviders();
        } catch (IOException e) {
            logger.error("Couldn't save service provider's config", e);
            throw new SibincoException("Couldn't save service provider's config", e);
        }
    }

    public synchronized void reloadServices(final SCAGAppContext appContext, final boolean isAdd, final long id, final ServiceProvider oldProvider) throws SCAGJspException {
      try {
          appContext.getScag().invokeCommand("reloadServices",null,appContext,this,configFilename);
      } catch (SibincoException e) {
          if (!(e instanceof StatusDisconnectedException)) {
              serviceProviders.remove(new Long(id));
              if (!isAdd) serviceProviders.put(oldProvider.getId(), oldProvider);
              throw new SCAGJspException(Constants.errors.serviceProviders.COULDNT_RELOAD_SERVICE_PROVIDER, e);
          }
      }
    }

    public synchronized void reloadServices(final SCAGAppContext appContext, final boolean isAdd, final long id, final Long serviceProviderId, final Service oldService) throws SCAGJspException {
      try {
          appContext.getScag().invokeCommand("reloadServices",null,appContext,this,configFilename);
      } catch (SibincoException e) {
          if (!(e instanceof StatusDisconnectedException)) {
              final ServiceProvider provider = (ServiceProvider) serviceProviders.get(serviceProviderId);
              provider.getServices().remove(new Long(id));
              if (!isAdd)  provider.getServices().put(oldService.getId(),oldService);
              //serviceProviders.put(provider.getId(), provider);
              throw new SCAGJspException(Constants.errors.serviceProviders.COULDNT_RELOAD_SERVICE_PROVIDER, e);
          }
      }
    }

    private void storeServiceProviders() throws IOException {
        File configFile = new File(configFilename);
        //File configNew = Functions.createNewFilenameForSave(configFile);

        PrintWriter out = new PrintWriter(new OutputStreamWriter(new FileOutputStream(configFile), Functions.getLocaleEncoding()));
        Functions.storeConfigHeader(out, "service_providers", "services.dtd", Functions.getLocaleEncoding());
        out.print(getParamXmlText());
        for (Iterator i = new SortedList(serviceProviders.keySet()).iterator(); i.hasNext();) {
            Long id = (Long) i.next();
            ServiceProvider serviceProvider = (ServiceProvider) serviceProviders.get(id);
            out.print(getXmlText(serviceProvider));
        }
        Functions.storeConfigFooter(out, "service_providers");
        out.flush();
        out.close();
        //Functions.renameNewSavedFileToOriginal(configNew, configFile);
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

    public synchronized ServiceProvider updateServiceProvider(final String user, final long id, final String name, final String description) throws NullPointerException {
        final ServiceProvider provider = (ServiceProvider) serviceProviders.get(new Long(id));
        final ServiceProvider oldProvider = (ServiceProvider)provider.copy();
        if (null == provider)
            throw new NullPointerException("Service Provider \"" + id + "\" not found.");
        provider.setName(name);
        if (description != null)
            provider.setDescription(description);
        StatMessage message = new StatMessage(user, "Service Provider", "Changed service provider: '" + name + "'");
        StatusManager.getInstance().addStatMessages(message);
        return oldProvider;
    }

    public synchronized long createServiceProvider(final String user, final String name, final String description) throws NullPointerException {
        final ServiceProvider serviceProvider = new ServiceProvider(new Long(++lastUsedServiceProviderId), name);
        serviceProvider.setDescription(description);
        serviceProviders.put(serviceProvider.getId(), serviceProvider);
        StatMessage message = new StatMessage(user, "Service Provider", "Added new service provider: '" + name + "'");
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
        StatMessage message = new StatMessage(user, "Service", "Added new service: '" + service.getName() + "'");
        StatusManager.getInstance().addStatMessages(message);
        return getLastUsedServiceId();
    }

    public synchronized void deleteServices(final String user, final List toRemove,
                                            final ServiceProvider serviceProvider, final SCAGAppContext appContext) throws SCAGJspException {
        List serviceNames = new ArrayList();
        Map servicesToDelete = new HashMap();
        for (Iterator it = toRemove.iterator(); it.hasNext();) {
            Long serviceId = (Long) it.next();
            Service service = (Service) serviceProvider.getServices().get(serviceId);
            serviceNames.add(service.getName());
            servicesToDelete.put(serviceId,service);
        }
        serviceProvider.getServices().keySet().removeAll(toRemove);
        try {
          appContext.getScag().invokeCommand("reloadServices",null,appContext,this,configFilename);
        } catch (SibincoException e) {
          if (!(e instanceof StatusDisconnectedException)) {
            serviceProvider.getServices().putAll(servicesToDelete);
            logger.error("Couldn't delete service(s) for service provider", e);
            throw new SCAGJspException(Constants.errors.serviceProviders.COULDNT_RELOAD_SERVICE_PROVIDER, e);
          }
        }
        StatMessage message = new StatMessage(user, "Service", "Deleted service(s): '" + serviceNames.toString() + "'");
        StatusManager.getInstance().addStatMessages(message);
    }

    public synchronized void deleteServiceProviders(final String user, final List toRemove, final SCAGAppContext appContext) throws SCAGJspException {
        List servProvNames = new ArrayList();
        Map providersToDelete = new HashMap();
        for (Iterator it = toRemove.iterator(); it.hasNext();) {
            Long id = (Long) it.next();
            ServiceProvider sp = (ServiceProvider) getServiceProviders().get(id);
            servProvNames.add(sp.getName());
            providersToDelete.put(id,sp);
        }
        getServiceProviders().keySet().removeAll(toRemove);
        try {
           appContext.getScag().invokeCommand("reloadServices",null,appContext,this,configFilename);
        } catch (SibincoException e) {
            if (!(e instanceof StatusDisconnectedException)) {
                //restore runtime storage
                getServiceProviders().putAll(providersToDelete);
                logger.error("Couldn't delete provider(s) ", e);
                throw new SCAGJspException(Constants.errors.serviceProviders.COULDNT_RELOAD_SERVICE_PROVIDER, e);
          }
        }
        StatMessage message = new StatMessage(user, "Service Provider", "Deleted service prvider(s): "
                + servProvNames.toString() + ".");
        StatusManager.getInstance().addStatMessages(message);
    }


    public synchronized long updateService(final String user, final long serviceProviderId, final Service service) throws NullPointerException {
        final ServiceProvider serviceProvider = (ServiceProvider) serviceProviders.get(new Long(serviceProviderId));
        if (null == serviceProvider)
            throw new NullPointerException("Service Provider \"" + serviceProvider + "\" not found.");
        serviceProvider.getServices().keySet().remove(service.getId());
        serviceProvider.getServices().put(service.getId(), service);
        serviceProviders.put(serviceProvider.getId(), serviceProvider);
        StatMessage message = new StatMessage(user, "Service", "Changed service: '" + service.getName() + "'");
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

    public synchronized Map getRoutesByServiceId(final Map routes, Long serviceId) {
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

    public synchronized Map getHttpRoutesByServiceId(final Map routes, Long serviceId) {
        Map result = Collections.synchronizedMap(new HashMap());
        for (Iterator i = new SortedList(routes.keySet()).iterator(); i.hasNext();) {
            String id = (String) i.next();
            HttpRoute route = (HttpRoute) routes.get(id);
            if (route.getService() != null) {
                if (route.getService().getId().equals(serviceId)) {
                    result.put(route.getId(), route);
                }
            }
        }
        return result;
    }

    public boolean isUniqueProviderName( String name, long id ){
        Map spMap = getServiceProviders();
        Iterator iterator = spMap.keySet().iterator();
        while( iterator. hasNext() ){
            ServiceProvider serviceProvider = (ServiceProvider)spMap.get( iterator.next() );
            if( serviceProvider.getName().equals(name) && serviceProvider.getId().longValue() != id){
                return false;
            }
        }
        return true;
    }

    public boolean isUniqueServiceName( String name, String id ){
        Map serviceProviderMap = getServiceProviders();
        Iterator serviceProviderIterator = serviceProviderMap.keySet().iterator();
        while( serviceProviderIterator. hasNext() ){
            ServiceProvider serviceProvider = (ServiceProvider)serviceProviderMap.get( serviceProviderIterator.next() );
            Map serviceMap = serviceProvider.getServices();
            Iterator serviceIterator = serviceMap.keySet().iterator();
            while( serviceIterator.hasNext() ){
                Long currentId = (Long)serviceIterator.next();
                Service service = getServiceById( currentId );
                if( service.getName().equals(name) && service.getId().longValue() != Long.parseLong(id) ){
                    return false;
                }
            }
        }
        return true;
    }
    public boolean checkForbiddenChars( String strToCheck ){
//        return strToCheck.contains( Constants.FORBIDDEN_CHARACTER )? false: true;
        return strToCheck.indexOf(Constants.FORBIDDEN_CHARACTER)!=-1? false: true;        
    }
}
