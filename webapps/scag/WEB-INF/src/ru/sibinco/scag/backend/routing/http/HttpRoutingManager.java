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
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.lib.backend.util.xml.Utils;
import ru.sibinco.scag.backend.service.ServiceProvidersManager;
import ru.sibinco.scag.backend.status.StatMessage;
import ru.sibinco.scag.backend.status.StatusManager;

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
public class HttpRoutingManager {

    private Logger logger = Logger.getLogger(this.getClass());

    public final static int HTTP_SUBJECT_TYPE = 0;
    public final static int HTTP_SITE_TYPE = 1;

    private Map routes = null;
    private Map subjects = null;
    private Map sites = null;
    private final File msagConfFolder;
    private ArrayList statMessages = new ArrayList();
    private ServiceProvidersManager serviceProvidersManager;
    private boolean routesChanged = false;
    private boolean routesRestored = false;
    private boolean routesSaved = false;
    private boolean routesLoaded = false;

    private static final String HTTP_ROUTES_PRIMARY_CONFIG = "http_routes.xml";
    private static final String HTTP_ROUTES_TEMPORAL_CONFIG = "http_routes_.xml";
    private static final String HTTP_ROUTES_TRACEABLE_CONFIG = "http_routes__.xml";


    public HttpRoutingManager(final File msagConfFolder, final ServiceProvidersManager serviceProvidersManager) {
        this.msagConfFolder = msagConfFolder;
        this.serviceProvidersManager = serviceProvidersManager;
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

    private void loadRoutes(NodeList routeList) throws SibincoException {
        routes = Collections.synchronizedMap(new HashMap());
        for (int i = 0; i < routeList.getLength(); i++) {
            final Element routeElem = (Element) routeList.item(i);
            String id = routeElem.getAttribute("id");
            routes.put(id, createRoute(routeElem, subjects));
        }
    }

    private HttpRoute createRoute(Element routeElem, Map subjects) throws SibincoException {
        return new HttpRoute(routeElem, subjects, serviceProvidersManager);
    }


    private void saveToFile(final String fileName) throws SibincoException {
        final File file = new File(msagConfFolder, fileName);
        final File newFile = Functions.createNewFilenameForSave(file);

        String localEncoding = Functions.getLocaleEncoding();
        if (localEncoding.equalsIgnoreCase("Cp1251")) {
            localEncoding = "WINDOWS-1251";
        }
        try {
            final PrintWriter out = new PrintWriter(new OutputStreamWriter(new FileOutputStream(newFile), localEncoding));
            Functions.storeConfigHeader(out, "http_routes", "http_routes.dtd", localEncoding);
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
            Functions.renameNewSavedFileToOriginal(newFile, file);

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

    public synchronized void save() throws SibincoException {
        saveToFile(HTTP_ROUTES_TEMPORAL_CONFIG);
    }

    public synchronized void restore() throws SibincoException {
        loadFromFile(HTTP_ROUTES_TEMPORAL_CONFIG);
        setRoutesRestored(true);
        clearStatMessages();
    }

    public synchronized void apply() throws SibincoException {
        saveToFile(HTTP_ROUTES_PRIMARY_CONFIG);
        clearStatMessages();
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
                System.out.println("lastModified = " + lastModified);
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

    public synchronized void deleteSubjectsSite(final String user, final Set checkedSet) {
        getSites().keySet().removeAll(checkedSet);
        setRoutesChanged(true);
        addStatMessages(new StatMessage(user, "Subject", "Deleted subject site(s): " + checkedSet.toString() + "."));
        StatusManager.getInstance().addStatMessages(new StatMessage(user,
                "Subject", "Deleted subject site(s): " + checkedSet.toString() + "."));
    }

    public synchronized void deleteSubjects(final String user, final Set checkedSet) {
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

    public Map getRoutes() {
        return routes;
    }

    public Map getSubjects() {
        return subjects;
    }

    public Map getSites() {
        return sites;
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
