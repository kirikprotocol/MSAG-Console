package ru.novosoft.smsc.admin.route;

/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 15:49:22
 */

import org.apache.log4j.Category;
import org.w3c.dom.Document;
import org.xml.sax.SAXException;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.smsc_service.RouteSubjectManager;
import ru.novosoft.smsc.admin.smsc_service.SmeManager;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.WebAppFolders;
import ru.novosoft.smsc.util.xml.Utils;

import javax.xml.parsers.FactoryConfigurationError;
import javax.xml.parsers.ParserConfigurationException;
import java.io.*;
import java.util.Date;
import java.util.Iterator;


public class RouteSubjectManagerImpl implements RouteSubjectManager {
    private RouteList routes = null;
    private SubjectList subjects = null;
    private Category logger = Category.getInstance(this.getClass());
    private SmeManager smeManager = null;

    private final static String SMSC_ROUTES_PRIMARY_CONFIG = "routes.xml";
    private final static String SMSC_ROUTES_TEMPORAL_CONFIG = "routes_.xml";
    private final static String SMSC_ROUTES_TRACEABLE_CONFIG = "routes__.xml";

    public RouteSubjectManagerImpl(SmeManager smeManager) throws AdminException {
        this.smeManager = smeManager;
        load();
        trace();
    }

    public RouteList getRoutes() {
        return routes;
    }

    public SubjectList getSubjects() {
        return subjects;
    }

    public int isSmeUsed(String smeId) {
        for (Iterator i = routes.iterator(); i.hasNext();) {
            Route route = (Route) i.next();
            if (route.getSrcSmeId().equals(smeId))
                return 1;
            if (route.getDestinations().isSmeUsed(smeId))
                return 1;
        }

        for (Iterator i = subjects.iterator(); i.hasNext();) {
            Subject subj = (Subject) i.next();
            if (subj.getDefaultSme().getId().equals(smeId))
                return 2;
        }

        return 0;
    }

    synchronized public boolean hasSavedConfiguration() {
        final File smscConfFolder = WebAppFolders.getSmscConfFolder();
        File config = new File(smscConfFolder, SMSC_ROUTES_TEMPORAL_CONFIG);
        return config.exists();
    }

    synchronized public void loadFromFile(String fileName) throws AdminException {
        try {
            final File smscConfFolder = WebAppFolders.getSmscConfFolder();
            File config = new File(smscConfFolder, fileName);

            Document routesDoc = Utils.parse(config.getAbsolutePath());
            subjects = new SubjectList(routesDoc.getDocumentElement(), smeManager);
            routes = new RouteList(routesDoc.getDocumentElement(), subjects, smeManager);
        } catch (FactoryConfigurationError error) {
            logger.error("Couldn't configure xml parser factory", error);
            throw new AdminException("Couldn't configure xml parser factory: " + error.getMessage());
        } catch (ParserConfigurationException e) {
            logger.error("Couldn't configure xml parser", e);
            throw new AdminException("Couldn't configure xml parser: " + e.getMessage());
        } catch (SAXException e) {
            logger.error("Couldn't parse", e);
            throw new AdminException("Couldn't parse: " + e.getMessage());
        } catch (IOException e) {
            logger.error("Couldn't perform IO operation", e);
            throw new AdminException("Couldn't perform IO operation: " + e.getMessage());
        } catch (NullPointerException e) {
            logger.error("Couldn't parse", e);
            throw new AdminException("Couldn't parse: " + e.getMessage());
        }
    }

    private void saveToFile(String filename) throws AdminException {
        try {
            final File file = new File(WebAppFolders.getSmscConfFolder(), filename);
            final File newFile = Functions.createNewFilenameForSave(file);
            PrintWriter out = new PrintWriter(new OutputStreamWriter(new FileOutputStream(newFile), Functions.getLocaleEncoding()));
            Functions.storeConfigHeader(out, "routes", "routes.dtd", Functions.getLocaleEncoding());
            subjects.store(out);
            routes.store(out);
            Functions.storeConfigFooter(out, "routes");
            out.flush();
            out.close();
            Functions.renameNewSavedFileToOriginal(newFile, file);
        } catch (FileNotFoundException e) {
            throw new AdminException("Couldn't save new routes settings: Couldn't write to destination config filename: " + e.getMessage());
        } catch (IOException e) {
            logger.error("Couldn't save new routes settings", e);
            throw new AdminException("Couldn't save new routes settings: " + e.getMessage());
        }
    }

    synchronized public void save() throws AdminException {
        saveToFile(SMSC_ROUTES_TEMPORAL_CONFIG);
    }

    synchronized public void restore() throws AdminException {
        loadFromFile(SMSC_ROUTES_TEMPORAL_CONFIG);
    }

    public Date getRestoreFileDate() throws AdminException // get modified date of temporal config
    {
        File tempConfFile = new File(WebAppFolders.getSmscConfFolder(), SMSC_ROUTES_TEMPORAL_CONFIG);
        if (tempConfFile.exists()) {
            final long lastModified = tempConfFile.lastModified();
            if (lastModified != 0) {
                logger.debug("RouteSubjectManagerImpl.getRestoreFileDate(): lastModified = " + lastModified);
                return new Date(lastModified);
            } else
                logger.debug("RouteSubjectManagerImpl.getRestoreFileDate(): tempConfFile.lastModified() == 0");
        } else
            logger.debug("RouteSubjectManagerImpl.getRestoreFileDate(): !tempConfFile.exists(), \"" + tempConfFile.getAbsolutePath() + '"');
        return null;
    }

    public Date getLoadFileDate() throws AdminException // get modified date of primary config
    {
        File tempConfFile = new File(WebAppFolders.getSmscConfFolder(), SMSC_ROUTES_PRIMARY_CONFIG);
        if (tempConfFile.exists()) {
            final long lastModified = tempConfFile.lastModified();
            if (lastModified != 0)
                return new Date(lastModified);
        }
        return null;
    }

    synchronized public void apply() throws AdminException {
        saveToFile(SMSC_ROUTES_PRIMARY_CONFIG);
    }

    synchronized public void load() throws AdminException {
        loadFromFile(SMSC_ROUTES_PRIMARY_CONFIG);
    }

    synchronized public void trace() throws AdminException {
        saveToFile(SMSC_ROUTES_TRACEABLE_CONFIG);
    }
}
