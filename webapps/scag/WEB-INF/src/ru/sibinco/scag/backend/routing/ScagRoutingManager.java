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
import ru.sibinco.lib.StatusDisconnectedException;
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.lib.backend.util.xml.Utils;
import ru.sibinco.scag.backend.endpoints.SmppManager;
import ru.sibinco.scag.backend.service.ServiceProvidersManager;
import ru.sibinco.scag.backend.status.StatMessage;
import ru.sibinco.scag.backend.status.StatusManager;
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
 * The <code>ScagRoutingManager</code> class represents
 */
public class ScagRoutingManager extends Manager {

    private Map<String, Route> routes = null;
    private Map<String, Subject> subjects = null;
    private final ArrayList<StatMessage> statMessages = new ArrayList<StatMessage>();
    private Logger logger = Logger.getLogger(this.getClass());
    private final File scagConfFolder;
    private SmppManager smppManager;
    private ServiceProvidersManager serviceProvidersManager;
    private HSDaemon hsDaemon;
    private boolean routesChanged = false;
    private String changedByUser = "";
    private boolean routesRestored = false;
    private boolean routesSaved = false;
    private boolean routesLoaded = false;

    private static final String SMPP_ROUTES_PRIMARY_CONFIG = "smpp_routes.xml";
    private static final String SMPP_ROUTES_TEMPORAL_CONFIG = "smpp_routes_.xml";
    private static final String SMPP_ROUTES_TRACEABLE_CONFIG = "smpp_routes__.xml";

    public ScagRoutingManager(File smscConfFolder, SmppManager smppManager,
                              ServiceProvidersManager serviceProvidersManager, HSDaemon hsDaemon) {
        this.scagConfFolder = smscConfFolder;
        this.smppManager = smppManager;
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

    /*public boolean isSmeUsed(final String svcId) {
        for (final Route route : routes.values())
            for (final Destination destination : route.getDestinations().values())
                if (destination.getSvc().getId().equals(svcId)) return true;
        return false;
    }*/

    public List getRoteIdsByServiceIds(String[] checked){
        final List<String> roteIds = new ArrayList<String>();
        for (final String serviceIdStr : checked) {
            final Long serviceId = Long.decode(serviceIdStr);
            Route[] routes = getRoutesByServiceId(serviceId);
            for (Route route : routes) roteIds.add(route.getId());
        }
        return roteIds;
    }

    public Route[] getRoutesByServiceId(final Long svcId) {
        List<Route> result = new ArrayList<Route>();
        for (final Route route : routes.values())
            if (route.getService().getId().equals(svcId)) result.add(route);

        return result.toArray(new Route[result.size()]);
    }

    public synchronized boolean hasSavedConfiguration() {
        return new File(scagConfFolder, SMPP_ROUTES_TEMPORAL_CONFIG).exists();
    }

    public synchronized void loadFromFile(final String fileName) throws SibincoException {

        logger.debug("enter " + this.getClass().getName() + ".loadFromFile(\"" + fileName + "\")");
        final File config = new File(scagConfFolder, fileName);

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

    public synchronized void deleteSubjects(final String user, final Set checkedSet) {
        getSubjects().keySet().removeAll(checkedSet);
        setRoutesChanged(true);
        addStatMessages(new StatMessage(user, "Subject", "Deleted subject(s): " + checkedSet.toString() + "."));
        StatusManager.getInstance().addStatMessages(new StatMessage(user,
                "Subject", "Deleted subject(s): " + checkedSet.toString() + "."));
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

    protected Subject createSubject(final Element subjElem, final SmppManager smppManager) throws SibincoException {
        return new Subject(subjElem, smppManager);
    }

    protected Route createRoute(final Element routeElem, final Map<String, Subject> subjects) throws SibincoException {
        return new Route(routeElem, subjects, smppManager, serviceProvidersManager);
    }

    private void loadSubjects(final NodeList subjList) throws SibincoException {
        subjects = Collections.synchronizedMap(new HashMap<String, Subject>());
        for (int i = 0; i < subjList.getLength(); i++) {
            final Element subjElem = (Element) subjList.item(i);
            subjects.put(subjElem.getAttribute("id"), createSubject(subjElem, smppManager));
        }
    }

    private void loadRoutes(final NodeList routeList) throws SibincoException {
        routes = Collections.synchronizedMap(new HashMap<String, Route>());
        for (int i = 0; i < routeList.getLength(); i++) {
            final Element routeElem = (Element) routeList.item(i);
            String id = routeElem.getAttribute("id");
            routes.put(id, createRoute(routeElem, subjects));
        }
    }

    private void saveToFile(final String filename) throws SibincoException {
       saveToFile(filename, true);
    }

    private void saveToFile(final String filename, boolean backup) throws SibincoException {
        // Check subjects.
        for (final Subject subject : subjects.values()) {
            if (logger.isInfoEnabled()) logger.info("not error, info for debug: " + subject);

            if (subject.getName() == null) {
                logger.error("Couldn't save default subject to file, subject name is null:" + subject);
                throw new SibincoException("Couldn't save default subject to file, subject name is null.");
            }

            if (subject.getCenter() == null && subject.getSvc() == null && subject.getMetaSvc() == null && subject.getMetaCenter() == null) {
                logger.error("Couldn't save default subject with name " + subject.getName() + " to file: subjects service endpoints and centers is null.");
                throw new SibincoException("Couldn't save default subject with name " + subject.getName() + " to file: all service endpoints and centers is null.");
            }

            if (subject.getCenter() != null) {
                if (subject.getCenter().getId() == null) {
                    logger.error("Couldn't save default subject with name " + subject.getName() + " to file, center id is null:" + subject);
                    throw new SibincoException("Couldn't save default subject with name " + subject.getName() + " to file: center id is null.");
                }
            }

            if (subject.getMetaCenter() != null) {
                if (subject.getMetaCenter().getId() == null) {
                    logger.error("Couldn't save default subject with name " + subject.getName() + " to file, meta center id is null:" + subject);
                    throw new SibincoException("Couldn't save default subject with name " + subject.getName() + " to file: meta center id is null.");
                }
            }

            if (subject.getSvc() != null) {
                if (subject.getSvc().getId() == null) {
                    logger.error("Couldn't save default subject with name " + subject.getName() + " to file, svc id is null:" + subject);
                    throw new SibincoException("Couldn't save default subject with name " + subject.getName() + " to file: svc id is null.");
                }
            }

            if (subject.getMetaSvc() != null) {
                if (subject.getMetaSvc().getId() == null) {
                    logger.error("Couldn't save default subject with name " + subject.getName() + " to file, meta svc id is null:" + subject);
                    throw new SibincoException("Couldn't save default subject with name " + subject.getName() + " to file: meta svc id is null.");
                }
            }

            MaskList masks = subject.getMasks();
            TreeSet<String> childSubjects = subject.getChildSubjects();
            if ((masks == null || masks.size() == 0) && (childSubjects == null || childSubjects.size() == 0)) {
                logger.error("Couldn't save default subject with name " + subject.getName() + " to file, masks and subjects are null or empty:" + subject);
                throw new SibincoException("Couldn't save default subject with name " + subject.getName() + " to file: masks is null.");
            }
        }
        if (logger.isDebugEnabled()) logger.debug("Subjects has been checked.");

        final File file = new File(scagConfFolder, filename);
        File newFile = null;
        try{
            if (backup) newFile = Functions.createNewFilenameForSave(file);
            if (logger.isDebugEnabled()) logger.debug("Successfully create backup file.");
        } catch (Throwable e){
            logger.error("Couldn't create new backup file.", e);
            throw new SibincoException("Couldn't save new routes settings",e);
        }

        String localEncoding;
        try{
            localEncoding = Functions.getLocaleEncoding();
            if (logger.isDebugEnabled()) logger.debug("Successfully get locale encoding.");
        } catch (Throwable e){
            logger.error("Couldn't get locale encoding.", e);
            throw new SibincoException("Couldn't save new routes settings",e);
        }

        FileOutputStream fos;
        try {
            fos = new FileOutputStream((backup)?newFile:file);
        } catch (FileNotFoundException e) {
            logger.error("Couldn't find file: "+ ((backup)?newFile:file), e);
            throw new SibincoException("Couldn't save new routes settings", e);
        }

        OutputStreamWriter osw;
        try {
            osw = new OutputStreamWriter(fos, localEncoding);
        } catch (UnsupportedEncodingException e) {
            logger.error(e);
            throw new SibincoException("Couldn't save new routes settings",e);
        }

        PrintWriter out = null;
        try {
            out = new PrintWriter(osw);
            try{
                Functions.storeConfigHeader(out, "routes", "smpp_routes.dtd", localEncoding);
            } catch (Throwable e){
                logger.error("Couldn't store config header.", e);
                throw new SibincoException("Couldn't save new routes settings", e);
            }

            for (final Subject subject : subjects.values()) {
                if (subject != null){
                    try{
                         subject.store(out);
                    } catch (Throwable e){
                        logger.error("Couldn't store subject: "+subject, e);
                        throw new SibincoException("Couldn't save new routes settings", e);
                    }
                }
            }

            for (final Route route : routes.values()) {
                if (route != null){
                    try{
                         route.store(out);
                    } catch (Throwable e){
                        logger.error("Couldn't store route: "+route.getName(), e);
                        throw new SibincoException("Couldn't save new routes settings", e);
                    }
                }
            }

            try{
                Functions.storeConfigFooter(out, "routes");
                if (logger.isDebugEnabled()) logger.debug("Config footer has been stored.");
            } catch (Throwable e){
                logger.error("Couldn't store config footer.", e);
                throw new SibincoException("Couldn't save new routes settings", e);
            }
            out.flush();
            out.close();

            try{
                if (backup) Functions.renameNewSavedFileToOriginal(newFile, file);
                if (logger.isDebugEnabled()) logger.debug("New saved file has been renamed to original.");
            } catch (Throwable e){
                logger.error("Couldn't rename new saved file to original.", e);
                throw new SibincoException("Couldn't rename new saved file to original.");
            }

        } finally {
            if (out!=null) out.close();
            try {
                osw.close();
            } catch (IOException e) {
                logger.error(e);
            }

            try {
                fos.close();
            } catch (IOException e) {
                logger.error(e);
            }
        }
    }

    /**
     * Method for get modified date of temporal config
     *
     * @return Date - modified date
     * @throws SibincoException if exception will be occurred
     */
    public Date getRestoreFileDate() throws SibincoException {
        File tempConfFile = new File(scagConfFolder, SMPP_ROUTES_TEMPORAL_CONFIG);
        if (tempConfFile.exists()) {
            final long lastModified = tempConfFile.lastModified();
            if (lastModified != 0) {
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
    /*public Date getLoadFileDate() throws SibincoException {
        File tempConfFile = new File(scagConfFolder, SMPP_ROUTES_PRIMARY_CONFIG);
        if (tempConfFile.exists()) {
            final long lastModified = tempConfFile.lastModified();
            if (lastModified != 0)
                return new Date(lastModified);
        }
        return null;
    }*/

    public int getProviderIdByServiceId(final Long serviceId) throws SibincoException {
      return (serviceProvidersManager.getServiceProviderByServiceId(serviceId)).getId().intValue();
    }

    public int getServiceIdByRouteId(final String routeId) throws SibincoException {
      final Route route = routes.get(routeId);
      if (route == null) return 0;
      return route.getService().getId().intValue();
    }

    public synchronized void save() throws SibincoException {
        saveToFile(SMPP_ROUTES_TEMPORAL_CONFIG);
        hsDaemon.store(scagConfFolder, SMPP_ROUTES_TEMPORAL_CONFIG);
    }

    public synchronized void restore() throws SibincoException {
        loadFromFile(SMPP_ROUTES_TEMPORAL_CONFIG);
        setRoutesRestored(true);
        clearStatMessages();
    }

    public synchronized void apply(final SCAGAppContext appContext) throws SCAGJspException {
      try {
          logger.debug("ScagRoutingManager.apply()");
          appContext.getScag().invokeCommand( "applySmppRoutes",null,appContext,this,new File(scagConfFolder, SMPP_ROUTES_PRIMARY_CONFIG).getAbsolutePath() );
      } catch (SibincoException e) {
          logger.error("ScagRoutingManager.apply() SibincoException");
          if( !(e instanceof StatusDisconnectedException) ) {
              logger.error("ScagRoutingManager.apply() SibincoException Couldn't apply routes", e);
              throw new SCAGJspException(Constants.errors.routing.routes.COULDNT_APPLY_ROUTES, e);
          } else{
              logger.error("ScagRoutingManager.apply() StatusDisconnectedException Couldn't apply routes", e);
              throw new SCAGJspException(Constants.errors.routing.routes.COULDNT_APPLY_ROUTES_SAVE_WV, e);
          }

      }
      clearStatMessages();
    }

    public void store() throws SibincoException {
       saveToFile(SMPP_ROUTES_PRIMARY_CONFIG,false);
    }

    public synchronized void load() throws SibincoException {
        loadFromFile(SMPP_ROUTES_PRIMARY_CONFIG);
        setRoutesLoaded(true);
        clearStatMessages();
    }

    public synchronized void trace() throws SibincoException {
        saveToFile(SMPP_ROUTES_TRACEABLE_CONFIG);
    }

    public Map<String, Route> getRoutes() {
        return routes;
    }

    public Map<String, Subject> getSubjects() {
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
