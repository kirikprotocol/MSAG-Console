/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.routing.http;

import org.apache.log4j.Logger;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.StatusDisconnectedException;
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.lib.backend.util.xml.Utils;
import ru.sibinco.scag.backend.service.ServiceProvidersManager;
import ru.sibinco.scag.backend.service.Service;
import ru.sibinco.scag.backend.status.StatMessage;
import ru.sibinco.scag.backend.status.StatusManager;
import ru.sibinco.scag.backend.routing.http.placement.Option;
import ru.sibinco.scag.backend.Manager;
import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.backend.installation.HSDaemon;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.Constants;

import javax.xml.parsers.FactoryConfigurationError;
import javax.xml.parsers.ParserConfigurationException;
import java.io.*;
import java.util.*;

/**
 * The <code>HttpRoutingManager</code> class represents
 * <p><p/>
 * Date: 06.05.2006
 * Time: 15:55:26
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class HttpRoutingManager extends Manager{

    private Logger logger = Logger.getLogger(this.getClass());

    public final static int HTTP_SUBJECT_TYPE = 0;
    public final static int HTTP_SITE_TYPE = 1;

    private Map routes = null;
    private Map subjects = null;
    private Map sites = null;
    private Option options = null;
    private final File msagConfFolder;
    private ArrayList statMessages = new ArrayList();
    private ServiceProvidersManager serviceProvidersManager;
    private HSDaemon hsDaemon;
    private boolean routesChanged = false;
    private boolean routesRestored = false;
    private boolean routesSaved = false;
    private boolean routesLoaded = false;

    private static final String HTTP_ROUTES_PRIMARY_CONFIG = "http_routes.xml";
    private static final String HTTP_ROUTES_TEMPORAL_CONFIG = "http_routes_.xml";
    private static final String HTTP_ROUTES_TRACEABLE_CONFIG = "http_routes__.xml";

    private long lastUsedHttpRouteId = -1;

    private static final String PARAM_NAME_LAST_USED_HTTP_ROUTE_ID = "last used http route id";

    public HttpRoutingManager(final File msagConfFolder, final ServiceProvidersManager serviceProvidersManager, final HSDaemon hsDaemon) {
        this.msagConfFolder = msagConfFolder;
        this.serviceProvidersManager = serviceProvidersManager;
        this.hsDaemon = hsDaemon;
    }

    public void init() throws SibincoException {
        try {
            load();
            //trace();
        } catch (SibincoException e) {
            e.printStackTrace();
            throw new SibincoException(e.getMessage());
        }
    }

    private void loadSubjects(final NodeList subjList) throws SibincoException {
        subjects = Collections.synchronizedMap(new HashMap());
        for (int i = 0; i < subjList.getLength(); i++) {
            final Element subjElem = (Element) subjList.item(i);
            subjects.put(subjElem.getAttribute("id"), createSubject(subjElem));
        }
    }

    private void loadSites(final NodeList siteList) throws SibincoException {
        sites = Collections.synchronizedMap(new HashMap());
        for (int i = 0; i < siteList.getLength(); i++) {
            final Element siteElement = (Element) siteList.item(i);
            sites.put(siteElement.getAttribute("id"), createSite(siteElement));
        }
    }

    public synchronized boolean hasSavedConfiguration() {
        return new File(msagConfFolder, HTTP_ROUTES_TEMPORAL_CONFIG).exists();
    }

    public synchronized void loadFromFile(final String fileName) throws SibincoException {
        logger.debug("enter " + this.getClass().getName() + ".loadFromFile(\"" + fileName + "\")");
        final File config = new File(msagConfFolder, fileName);
        logger.debug("exit " + this.getClass().getName() + ".loadFromFile(\"" + fileName + "\")");
        try {
            final Document routesDocument = Utils.parse(config.getAbsolutePath());
            loadParams(routesDocument.getDocumentElement().getElementsByTagName("param"));
            loadOptions(routesDocument.getDocumentElement().getElementsByTagName("options"));
            loadSubjects(routesDocument.getDocumentElement().getElementsByTagName("subject_def"));
            loadSites(routesDocument.getDocumentElement().getElementsByTagName("site_subject_def"));
            loadRoutes(routesDocument.getDocumentElement().getElementsByTagName("route"));
        } catch (FactoryConfigurationError error) {
            logger.error("Couldn't configure xml parser factory", error);
            throw new SibincoException("Couldn't configure xml parser factory", error);
        } catch (ParserConfigurationException e) {
            logger.error("Couldn't configure xml parser", e);
            throw new SibincoException("Couldn't configure xml parser", e);
        } catch (SAXException e) {
            logger.error("Couldn't parse", e);
            throw new SibincoException("Couldn't parse", e);
        } catch (IOException e) {
            logger.error("Couldn't perfmon IO operation", e);
            throw new SibincoException("Couldn't perfmon IO operation", e);
        } catch (NullPointerException e) {
            logger.error("Couldn't parse", e);
            throw new SibincoException("Couldn't parse", e);
        }
    }

    private void loadParams(NodeList params) {
        for (int i=0; i<params.getLength();i++) {
          final Element paramElement = (Element)params.item(i);
          final String name = paramElement.getAttribute("name");
          try {
            if (name.equals(PARAM_NAME_LAST_USED_HTTP_ROUTE_ID)) {
              lastUsedHttpRouteId = Integer.decode(Utils.getNodeText(paramElement)).intValue();
            }
          } catch (NumberFormatException e) {
            logger.error("Int parameter \"" + name + "\" misformatted: " + lastUsedHttpRouteId + ", skipped", e);
          }
        }
    }

    private void loadOptions(NodeList options) throws SibincoException {
        setOptions(new Option(options));
    }

    private void loadRoutes(NodeList routeList) throws SibincoException {
        routes = Collections.synchronizedMap(new HashMap());
        for (int i = 0; i < routeList.getLength(); i++) {
            final Element routeElem = (Element) routeList.item(i);
            String id = routeElem.getAttribute("id");
            routes.put(id, createRoute(id,routeElem, subjects));
        }
    }

    public HttpRoute createRoute(String name, Service serviceObj, boolean enabled, boolean defaultRoute,
                                 boolean transit, Abonent abonent, RouteSite routeSite) throws SibincoException {
      return new HttpRoute(new Long(++lastUsedHttpRouteId), name, serviceObj, enabled, defaultRoute, transit, abonent, routeSite);
    }

    public HttpRoute createRoute(String name, Service serviceObj, boolean enabled, boolean defaultRoute,
                                 boolean transit, Abonent abonent, RouteSite routeSite, boolean saa) throws SibincoException {
      return new HttpRoute(new Long(++lastUsedHttpRouteId), name, serviceObj, enabled, defaultRoute, transit, abonent, routeSite, saa);
    }

    private HttpRoute createRoute(String id, Element routeElem, Map subjects) throws SibincoException {
        return new HttpRoute(Long.valueOf(id), routeElem, subjects, serviceProvidersManager);
    }

    public boolean isDefaultRoute(Long serviceId) {
        return getRoutesByServiceId(serviceId).length>0?false:true;
    }

    public synchronized void setDefaultHttpRoute(final String user, final Set checkedSet, final Long serviceId) {
        if (checkedSet.size()==0) return;
        String id = (String)checkedSet.iterator().next();
        HttpRoute[] httpRoutes = getRoutesByServiceId(serviceId);
        //unchecking previous default for service
        HttpRoute cur;
        for (int i=0; i<httpRoutes.length;i++) {
           ((HttpRoute)httpRoutes[i]).setDefaultRoute(false);
        }
        //checking new default route
        ((HttpRoute)routes.get(id)).setDefaultRoute(true);
        setRoutesChanged(true);
        StatMessage message = new StatMessage(user, "Routes", "Set default route: " + checkedSet.toString() + " for service " + serviceId);
        StatusManager.getInstance().addStatMessages(message);
        addStatMessages(message);
    }

    private String getParamXmlText() {
        StringBuffer buffer = new StringBuffer();
        buffer.append("   <param name=\"" + PARAM_NAME_LAST_USED_HTTP_ROUTE_ID + "\" type=\"int\">").
               append(lastUsedHttpRouteId).append("</param>").append("\n");
        return buffer.toString();
    }

    private void saveToFile(final String filename) throws SibincoException {
      saveToFile(filename, true);
    }

    private void saveToFile(final String fileName, final boolean backup) throws SibincoException {
        final File file = new File(msagConfFolder, fileName);
        File newFile = null;
        if (backup) newFile = Functions.createNewFilenameForSave(file);

        String localEncoding = Functions.getLocaleEncoding();

        try {
            final PrintWriter out = new PrintWriter(new OutputStreamWriter(new FileOutputStream((backup)?newFile:file), localEncoding));
            Functions.storeConfigHeader(out, "http_routes", "http_routes.dtd", localEncoding);
            out.print(getParamXmlText());
            options.store(out);
            for (Iterator iterator = subjects.values().iterator(); iterator.hasNext();) {
                final HttpSubject httpSubject = (HttpSubject) iterator.next();
                httpSubject.store(out);
            }
            for (Iterator iterator = sites.values().iterator(); iterator.hasNext();) {
                final HttpSite httpSite = (HttpSite) iterator.next();
                httpSite.store(out);
            }
            for (Iterator iterator = routes.values().iterator(); iterator.hasNext();) {
                final HttpRoute httpRoute = (HttpRoute) iterator.next();
                httpRoute.store(out);
            }
            Functions.storeConfigFooter(out, "http_routes");
            out.flush();
            out.close();
            if (backup) Functions.renameNewSavedFileToOriginal(newFile, file);

        } catch (FileNotFoundException e) {
            throw new SibincoException("Couldn't save new routes settings: Couldn't write to destination config filename: " + e.getMessage());
        } catch (IOException e) {
            logger.error("Couldn't save new routes settings", e);
            throw new SibincoException("Couldn't save new routes settings", e);
        }
    }

    private HttpSubject createSubject(Element subjElem) throws SibincoException {
        return new HttpSubject(subjElem);
    }

    private HttpSite createSite(Element siteElem) throws SibincoException {
        return new HttpSite(siteElem);
    }

    public int getServiceIdByRouteId(final String routeId) throws SibincoException {
      final HttpRoute route = (HttpRoute)routes.get(routeId);
      if (route == null) return 0;
      return route.getService().getId().intValue();
    }

    public synchronized void save() throws SibincoException {
        saveToFile(HTTP_ROUTES_TEMPORAL_CONFIG);
        hsDaemon.store(msagConfFolder, HTTP_ROUTES_TEMPORAL_CONFIG);
    }

    public synchronized void restore() throws SibincoException {
        loadFromFile(HTTP_ROUTES_TEMPORAL_CONFIG);
        setRoutesRestored(true);
        clearStatMessages();
    }

    public synchronized void apply(final SCAGAppContext appContext) throws SCAGJspException {
      try {
          appContext.getScag().invokeCommand("applyHttpRoutes",null,appContext,this,new File(msagConfFolder, HTTP_ROUTES_PRIMARY_CONFIG).getAbsolutePath());
      } catch (SibincoException e) {
          if (!(e instanceof StatusDisconnectedException)) {
              logger.error("SibincoException Couldn't apply http routes", e);
              throw new SCAGJspException(Constants.errors.routing.routes.COULDNT_APPLY_HTTP_ROUTES, e);
          }
      }
      clearStatMessages();
    }

    public void store() throws SibincoException {
       saveToFile(HTTP_ROUTES_PRIMARY_CONFIG,false);
    }

    public synchronized void load() throws SibincoException {
        loadFromFile(HTTP_ROUTES_PRIMARY_CONFIG);
        setRoutesLoaded(true);
        clearStatMessages();
    }

    public synchronized void trace() throws SibincoException {
        saveToFile(HTTP_ROUTES_TRACEABLE_CONFIG);
    }

    /**
     * Method for get modified date of temporal config
     *
     * @return Date - modified date
     * @throws SibincoException if exception will be occurred
     */
    public Date getRestoreFileDate() throws SibincoException {
        File tempConfFile = new File(msagConfFolder, HTTP_ROUTES_TEMPORAL_CONFIG);
        if (tempConfFile.exists()) {
            final long lastModified = tempConfFile.lastModified();
            if (lastModified != 0) {
                //System.out.println("lastModified = " + lastModified);
                return new Date(lastModified);
            } else
                System.out.println("tempConfFile.lastModified() == 0");
        } else
            System.out.println("!tempConfFile.exists(), \"" + tempConfFile.getAbsolutePath() + '"');
        return null;
    }

    public synchronized void deleteSite(final String siteId, final String user, final Set checkedSet) {
        HttpSite httpSite = (HttpSite) getSites().get(siteId);
        if (httpSite != null) {
            httpSite.getSites().keySet().removeAll(checkedSet);
            setRoutesChanged(true);
            addStatMessages(new StatMessage(user, "Subject", "Deleted site(s): " + checkedSet.toString() + "."));
            StatusManager.getInstance().addStatMessages(new StatMessage(user,
                    "Subject", "Deleted site(s): " + checkedSet.toString() + "."));
        }
    }

    public synchronized void deleteSubjectsSite(final String user, final Set checkedSet) throws SCAGJspException{
        String subj;
        for (Iterator i=getRoutes().values().iterator();i.hasNext();) {
          HttpRoute route = (HttpRoute)i.next();
          RouteSite routeSite = route.getRouteSite();
          for(Iterator ii = checkedSet.iterator();ii.hasNext();) {
            subj = (String)ii.next();
            if (routeSite.getSiteSubjects().get(subj) !=null) throw new SCAGJspException(Constants.errors.routing.subjects.COULD_NOT_DELETE_SUBJECT_IS_BOUND," Subject \""+subj+"\" to route \""+route.getName()+"\"");
          }
        }
        getSites().keySet().removeAll(checkedSet);
        setRoutesChanged(true);
        addStatMessages(new StatMessage(user, "Subject", "Deleted subject site(s): " + checkedSet.toString() + "."));
        StatusManager.getInstance().addStatMessages(new StatMessage(user,
                "Subject", "Deleted subject site(s): " + checkedSet.toString() + "."));
    }

    public synchronized void deleteSubjects(final String user, final Set checkedSet) throws SCAGJspException{
        String subj;
        for (Iterator i=getRoutes().values().iterator();i.hasNext();) {
          HttpRoute route = (HttpRoute)i.next();
          Abonent abonent = route.getAbonent() ;
          for(Iterator ii = checkedSet.iterator();ii.hasNext();) {
            subj = (String)ii.next();
            if (abonent.getSubjects().get(subj) !=null) throw new SCAGJspException(Constants.errors.routing.subjects.COULD_NOT_DELETE_SUBJECT_IS_BOUND," Subject \""+subj+"\" to route \""+route.getName()+"\"");
          }
        }

        getSubjects().keySet().removeAll(checkedSet);
        setRoutesChanged(true);
        addStatMessages(new StatMessage(user, "Subject", "Deleted http subject(s): " + checkedSet.toString() + "."));
        StatusManager.getInstance().addStatMessages(new StatMessage(user,
                "Subject", "Deleted subject(s): " + checkedSet.toString() + "."));
    }

    /**
     * Method for get modified date of primary config
     *
     * @return Date - modified date
     * @throws SibincoException if exception will be occurred
     */
    public Date getLoadFileDate() throws SibincoException {
        File tempConfFile = new File(msagConfFolder, HTTP_ROUTES_PRIMARY_CONFIG);
        if (tempConfFile.exists()) {
            final long lastModified = tempConfFile.lastModified();
            if (lastModified != 0)
                return new Date(lastModified);
        }
        return null;
    }

    public List getRoteIdsByServiceIds(String[] checked){
        final List roteIds = new ArrayList();
        for (int i = 0; i < checked.length; i++) {
            final String serviceIdStr = checked[i];
            final Long serviceId = Long.decode(serviceIdStr);
            HttpRoute[] routes = getRoutesByServiceId(serviceId);
            for (int j = 0; j < routes.length; j++) {
                roteIds.add(routes[j].getId());
            }
        }
        return roteIds;
    }

    public HttpRoute[] getRoutesByServiceId(final Long svcId) {
        List result = new ArrayList();
        for (Iterator it = routes.values().iterator(); it.hasNext();) {
            final HttpRoute route = (HttpRoute) it.next();
            if(route != null && route.getService().getId().equals(svcId)){
               result.add(route);
            }
        }
        return (HttpRoute[]) result.toArray(new HttpRoute[result.size()]);
    }

     public synchronized void deleteRoutes(final String user, final Set checkedSet) {
        getRoutes().keySet().removeAll(checkedSet);
        setRoutesChanged(true);
        StatMessage message = new StatMessage(user, "Routes", "Deleted route(s): " + checkedSet.toString() + ".");
        StatusManager.getInstance().addStatMessages(message);
        addStatMessages(message);
    }

    public synchronized void deleteRoutes(final String user, final List checkedSet) {
        getRoutes().keySet().removeAll(checkedSet);
        setRoutesChanged(true);
        StatMessage message = new StatMessage(user, "Routes", "Deleted route(s): " + checkedSet.toString() + ".");
        StatusManager.getInstance().addStatMessages(message);
        addStatMessages(message);
    }

    public synchronized HttpSite getHttpSiteByName(String name) {
        return (HttpSite) sites.get(name);
    }

    public synchronized String getHttpRouteNameById(String id) {
        HttpRoute route = (HttpRoute)routes.get(id);
        if (route !=null)
          return route.getName();
        else
          return "unknown";
    }

    public Map getRoutes() {
        return routes;
    }

    public Map getSubjects() {
        return subjects;
    }

    public Map getSites() {
        return sites;
    }

    public synchronized Option getOptions() {
        return options;
    }

    public synchronized void setOptions(Option options) {
        this.options = options;
    }

    public boolean isRoutesChanged() {
        return routesChanged;
    }

    public void setRoutesChanged(boolean routesChanged) {
        this.routesChanged = routesChanged;
    }

    public boolean isRoutesRestored() {
        return routesRestored;
    }

    public void setRoutesRestored(boolean routesRestored) {
        this.routesRestored = routesRestored;
    }

    public boolean isRoutesSaved() {
        return routesSaved;
    }

    public void setRoutesSaved(boolean routesSaved) {
        this.routesSaved = routesSaved;
    }

    public boolean isRoutesLoaded() {
        return routesLoaded;
    }

    public void setRoutesLoaded(boolean routesLoaded) {
        this.routesLoaded = routesLoaded;
    }

    public synchronized ArrayList getStatMessages() {
        return statMessages;
    }

    public synchronized void addStatMessages(StatMessage message) {
        statMessages.add(message);
    }

    public synchronized void clearStatMessages() {
        statMessages.clear();
    }
}
