/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 15:49:22
 */
package ru.sibinco.lib.backend.route;

import org.apache.log4j.Logger;
import org.w3c.dom.*;
import org.xml.sax.SAXException;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.sme.SmeManager;
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.lib.backend.util.xml.Utils;

import javax.xml.parsers.FactoryConfigurationError;
import javax.xml.parsers.ParserConfigurationException;
import java.io.*;
import java.util.*;


public abstract class RouteSubjectManager
{
  private final Map routes = Collections.synchronizedMap(new HashMap());
  private final Map subjects = Collections.synchronizedMap(new HashMap());
  private final Logger logger = Logger.getLogger(this.getClass());
  private final File smscConfFolder;
  private final SmeManager smeManager;

  private static final String SMSC_ROUTES_PRIMARY_CONFIG = "routes.xml";
  private static final String SMSC_ROUTES_TEMPORAL_CONFIG = "routes_.xml";
  private static final String SMSC_ROUTES_TRACEABLE_CONFIG = "routes__.xml";

  public RouteSubjectManager(final File smscConfFolder, final SmeManager smeManager)
  {
    this.smscConfFolder = smscConfFolder;
    this.smeManager = smeManager;
  }

  public void init()
      throws SibincoException
  {
    load();
    trace();
  }

  public Map getRoutes()
  {
    return routes;
  }

  public Map getSubjects()
  {
    return subjects;
  }

  public boolean isSmeUsed(final String smeId)
  {
    for (Iterator i = routes.values().iterator(); i.hasNext();) {
      final Route route = (Route) i.next();
      for (Iterator j = route.getDestinations().values().iterator(); j.hasNext();) {
        final Destination destination = (Destination) j.next();
        if (destination.getSme().getId().equals(smeId))
          return true;
      }
    }

    return false;
  }

  public synchronized boolean hasSavedConfiguration()
  {
    return new File(smscConfFolder, SMSC_ROUTES_TEMPORAL_CONFIG).exists();
  }

  public synchronized void loadFromFile(final String fileName) throws SibincoException
  {
    try {
      logger.debug("enter " + this.getClass().getName() + ".loadFromFile(\"" + fileName + "\")");
      final File config = new File(smscConfFolder, fileName);

      final Document routesDoc = Utils.parse(config.getAbsolutePath());
      loadSubjects(routesDoc.getDocumentElement().getElementsByTagName("subject_def"), smeManager);
      loadRoutes(routesDoc.getDocumentElement().getElementsByTagName("route"));
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
      logger.error("Couldn't perform IO operation", e);
      throw new SibincoException("Couldn't perform IO operation", e);
    } catch (NullPointerException e) {
      logger.error("Couldn't parse", e);
      throw new SibincoException("Couldn't parse", e);
    }
  }

  private void loadSubjects(final NodeList subjList, final SmeManager smeManager) throws SibincoException
  {
    for (int i = 0; i < subjList.getLength(); i++) {
      final Element subjElem = (Element) subjList.item(i);
      subjects.put(subjElem.getAttribute("id"), createSubject(subjElem, smeManager));
    }
  }

  protected abstract Subject createSubject(Element subjElem, SmeManager smeManager) throws SibincoException;

  private void loadRoutes(final NodeList routeList) throws SibincoException
  {
    for (int i = 0; i < routeList.getLength(); i++) {
      final Element routeElem = (Element) routeList.item(i);
      routes.put(routeElem.getAttribute("id"), createRoute(routeElem, subjects, smeManager));
    }
  }

  protected abstract Route createRoute(Element routeElem, Map subjects, SmeManager smeManager) throws SibincoException;


  private void saveToFile(final String filename) throws SibincoException
  {
    try {
      final File file = new File(smscConfFolder, filename);
      final File newFile = Functions.createNewFilenameForSave(file);
      final PrintWriter out = new PrintWriter(new OutputStreamWriter(new FileOutputStream(newFile), Functions.getLocaleEncoding()));
      Functions.storeConfigHeader(out, "routes", "routes.dtd", Functions.getLocaleEncoding());
      for (Iterator i = subjects.values().iterator(); i.hasNext();) {
        final Subject subject = (Subject) i.next();
        subject.store(out);
      }
      for (Iterator i = routes.values().iterator(); i.hasNext();) {
        final Route route = (Route) i.next();
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

  public synchronized void save() throws SibincoException
  {
    saveToFile(SMSC_ROUTES_TEMPORAL_CONFIG);
  }

  public synchronized void restore() throws SibincoException
  {
    loadFromFile(SMSC_ROUTES_TEMPORAL_CONFIG);
  }

  public synchronized void apply() throws SibincoException
  {
    saveToFile(SMSC_ROUTES_PRIMARY_CONFIG);
  }

  public synchronized void load() throws SibincoException
  {
    loadFromFile(SMSC_ROUTES_PRIMARY_CONFIG);
  }

  public synchronized void trace() throws SibincoException
  {
    saveToFile(SMSC_ROUTES_TRACEABLE_CONFIG);
  }
}
