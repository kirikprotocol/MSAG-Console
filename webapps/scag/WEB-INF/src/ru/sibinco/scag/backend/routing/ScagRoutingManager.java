/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.backend.routing;

import org.apache.log4j.Logger;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.lib.backend.util.xml.Utils;
import ru.sibinco.scag.backend.endpoints.SmppManager;
import ru.sibinco.scag.backend.rules.RuleManager;
import ru.sibinco.scag.backend.sme.CategoryManager;
import ru.sibinco.scag.backend.sme.ProviderManager;

import javax.xml.parsers.FactoryConfigurationError;
import javax.xml.parsers.ParserConfigurationException;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;


/**
 * The <code>ScagRoutingManager</code> class represents
 * <p><p/>
 * Date: 21.10.2005
 * Time: 15:00:19
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class ScagRoutingManager {

    private Map routes = null;//Collections.synchronizedMap(new HashMap());
    private Map subjects = null;//Collections.synchronizedMap(new HashMap());
    private Logger logger = Logger.getLogger(this.getClass());
    private final File smscConfFolder;
    private SmppManager smppManager;
    private ProviderManager providerManager;
    private RuleManager ruleManager;
    private CategoryManager categoryManager;
    private boolean routesChanged = false;
    private String changedByUser = "";
    private boolean routesRestored = false;
    private boolean routesSaved = false;
    private boolean routesLoaded = false;

    private static final String SMSC_ROUTES_PRIMARY_CONFIG = "routes.xml";
    private static final String SMSC_ROUTES_TEMPORAL_CONFIG = "routes_.xml";
    private static final String SMSC_ROUTES_TRACEABLE_CONFIG = "routes__.xml";


    public ScagRoutingManager(File smscConfFolder, SmppManager smppManager,
                              ProviderManager providerManager, RuleManager ruleManager, CategoryManager categoryManager) {
        this.smscConfFolder = smscConfFolder;
        this.smppManager = smppManager;
        this.providerManager = providerManager;
        this.ruleManager = ruleManager;
        this.categoryManager = categoryManager;
    }

    public void init() throws SibincoException {
        try {
            load();
            trace();
        } catch (SibincoException e) {
            e.printStackTrace();
            throw new SibincoException(e.getMessage());
        }
    }

    public boolean isSmeUsed(final String svcId) {
        for (Iterator it = routes.values().iterator(); it.hasNext();) {
            final Route route = (Route) it.next();
            for (Iterator j = route.getDestinations().values().iterator(); j.hasNext();) {
                final Destination destination = (Destination) j.next();
                if (destination.getSvc().getId().equals(svcId))
                    return true;
            }
        }
        return false;
    }

    public synchronized boolean hasSavedConfiguration() {
        return new File(smscConfFolder, SMSC_ROUTES_TEMPORAL_CONFIG).exists();
    }

    public synchronized void loadFromFile(final String fileName) throws SibincoException {

        logger.debug("enter " + this.getClass().getName() + ".loadFromFile(\"" + fileName + "\")");
        final File config = new File(smscConfFolder, fileName);

        try {
            final Document routesDocument = Utils.parse(config.getAbsolutePath());
            loadSubjects(routesDocument.getDocumentElement().getElementsByTagName("subject_def"));
            loadRoutes(routesDocument.getDocumentElement().getElementsByTagName("route"));
            logger.debug("exit " + this.getClass().getName() + ".loadFromFile(\"" + fileName + "\")");
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

    protected Subject createSubject(final Element subjElem, final SmppManager smppManager) throws SibincoException {
        return new Subject(subjElem, smppManager);
    }

    protected Route createRoute(final Element routeElem, final Map subjects) throws SibincoException {
        return new Route(routeElem, subjects, smppManager, ruleManager, providerManager, categoryManager);
    }

    private void loadSubjects(final NodeList subjList) throws SibincoException {
        subjects = Collections.synchronizedMap(new HashMap());
        for (int i = 0; i < subjList.getLength(); i++) {
            final Element subjElem = (Element) subjList.item(i);
            subjects.put(subjElem.getAttribute("id"), createSubject(subjElem, smppManager));
        }
    }

    private void loadRoutes(final NodeList routeList) throws SibincoException {
        routes = Collections.synchronizedMap(new HashMap());
        for (int i = 0; i < routeList.getLength(); i++) {
            final Element routeElem = (Element) routeList.item(i);
            String id = routeElem.getAttribute("id");
            routes.put(id, createRoute(routeElem, subjects));
        }
    }

    private void saveToFile(final String filename) throws SibincoException {

        final File file = new File(smscConfFolder, filename);
        final File newFile = Functions.createNewFilenameForSave(file);
        try {
            String localEncoding = Functions.getLocaleEncoding();
            if (localEncoding.equalsIgnoreCase("Cp1251"))
                localEncoding = "WINDOWS-1251";
            final PrintWriter out = new PrintWriter(new OutputStreamWriter(new FileOutputStream(newFile), localEncoding));

            Functions.storeConfigHeader(out, "routes", "routes.dtd", localEncoding);
            for (Iterator it = subjects.values().iterator(); it.hasNext();) {
                final Subject subject = (Subject) it.next();
                subject.store(out);
            }
            for (Iterator it = routes.values().iterator(); it.hasNext();) {
                final Route route = (Route) it.next();
                route.store(out);
            }
            Functions.storeConfigFooter(out, "routes");
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

    /**
     * Method for get modified date of temporal config
     *
     * @return Date - modified date
     * @throws SibincoException if exception will be occurred
     */
    public Date getRestoreFileDate() throws SibincoException {
        File tempConfFile = new File(smscConfFolder, SMSC_ROUTES_TEMPORAL_CONFIG);
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

    /**
     * Method for get modified date of primary config
     *
     * @return Date - modified date
     * @throws SibincoException if exception will be occurred
     */
    public Date getLoadFileDate() throws SibincoException {
        File tempConfFile = new File(smscConfFolder, SMSC_ROUTES_PRIMARY_CONFIG);
        if (tempConfFile.exists()) {
            final long lastModified = tempConfFile.lastModified();
            if (lastModified != 0)
                return new Date(lastModified);
        }
        return null;
    }

    public synchronized void save() throws SibincoException {
        saveToFile(SMSC_ROUTES_TEMPORAL_CONFIG);
    }

    public synchronized void restore() throws SibincoException {
        loadFromFile(SMSC_ROUTES_TEMPORAL_CONFIG);
        setRoutesRestored(true);
    }

    public synchronized void apply() throws SibincoException {
        saveToFile(SMSC_ROUTES_PRIMARY_CONFIG);
    }

    public synchronized void load() throws SibincoException {
        loadFromFile(SMSC_ROUTES_PRIMARY_CONFIG);
        setRoutesLoaded(true);
    }

    public synchronized void trace() throws SibincoException {
        saveToFile(SMSC_ROUTES_TRACEABLE_CONFIG);
    }


    public Map getRoutes() {
        return routes;
    }

    public Map getSubjects() {
        return subjects;
    }

    public boolean isRoutesChanged() {
        return routesChanged;
    }

    public void setRoutesChanged(final boolean routesChanged) {
        this.routesChanged = routesChanged;
    }

    public String getChangedByUser() {
        return changedByUser;
    }

    public void setChangedByUser(String changedByUser) {
        this.changedByUser = changedByUser;
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

}
