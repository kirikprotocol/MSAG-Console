/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 15:49:22
 */
package ru.novosoft.smsc.admin.route;

import org.apache.log4j.Category;
import org.w3c.dom.Document;
import org.xml.sax.SAXException;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.smsc_service.RouteSubjectManager;
import ru.novosoft.smsc.admin.smsc_service.SmeManager;
import ru.novosoft.smsc.util.WebAppFolders;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.xml.Utils;

import javax.xml.parsers.FactoryConfigurationError;
import javax.xml.parsers.ParserConfigurationException;
import java.io.*;
import java.util.Iterator;


public class RouteSubjectManagerImpl implements RouteSubjectManager
{
	private RouteList routes = null;
	private SubjectList subjects = null;
	private Category logger = Category.getInstance(this.getClass());
  private SmeManager smeManager = null;

  private final static String SMSC_ROUTES_PRIMARY_CONFIG = "routes.xml";
  private final static String SMSC_ROUTES_TEMPORAL_CONFIG = "routes_.xml";
  private final static String SMSC_ROUTES_TRACEABLE_CONFIG = "routes__.xml";

	public RouteSubjectManagerImpl(SmeManager smeManager) throws AdminException
	{
    this.smeManager = smeManager;
    load(); trace();
	}

	public RouteList getRoutes() {
		return routes;
	}

	public SubjectList getSubjects() {
		return subjects;
	}

	public boolean isSmeUsed(String smeId)
	{
		for (Iterator i = routes.iterator(); i.hasNext();)
		{
			Route route = (Route) i.next();
			if (route.getDestinations().isSmeUsed(smeId))
				return true;
		}
		return false;
	}

  synchronized public boolean hasSavedConfiguration()
  {
    final File smscConfFolder = WebAppFolders.getSmscConfFolder();
    File config = new File(smscConfFolder, SMSC_ROUTES_TEMPORAL_CONFIG);
    return config.exists();
  }

  /*private void copyFile(File source, File destination)
      throws IOException
  {
    if (destination.exists()) destination.delete();
    destination.createNewFile();

    FileInputStream  is = new FileInputStream(source);
    FileOutputStream os = new FileOutputStream(destination);
    while (is.available() > 0) {
      int b = is.read(); if (b == -1) break; os.write(b);
    }
    os.flush(); os.close(); is.close();
  }*/

  synchronized public void loadFromFile(String fileName) throws AdminException
  {
    try
    {
      final File smscConfFolder = WebAppFolders.getSmscConfFolder();
      File config = new File(smscConfFolder, fileName);

      Document routesDoc = Utils.parse(new FileReader(config));
      subjects = new SubjectList(routesDoc.getDocumentElement(), smeManager);
      routes = new RouteList(routesDoc.getDocumentElement(), subjects, smeManager);
    }
    catch (FactoryConfigurationError error)
    {
      logger.error("Couldn't configure xml parser factory", error);
      throw new AdminException("Couldn't configure xml parser factory: " + error.getMessage());
    }
    catch (ParserConfigurationException e)
    {
      logger.error("Couldn't configure xml parser", e);
      throw new AdminException("Couldn't configure xml parser: " + e.getMessage());
    }
    catch (SAXException e)
    {
      logger.error("Couldn't parse", e);
      throw new AdminException("Couldn't parse: " + e.getMessage());
    }
    catch (IOException e)
    {
      logger.error("Couldn't perform IO operation", e);
      throw new AdminException("Couldn't perform IO operation: " + e.getMessage());
    }
    catch (NullPointerException e)
    {
      logger.error("Couldn't parse", e);
      throw new AdminException("Couldn't parse: " + e.getMessage());
    }
  }

  private void saveToFile(String filename) throws AdminException
  {
    try
    {
      final File file = new File(WebAppFolders.getSmscConfFolder(), filename);
      final File newFile = Functions.createNewFilenameForSave(file);
      PrintWriter out = new PrintWriter(new FileWriter(newFile));
      Functions.storeConfigHeader(out, "routes", "routes.dtd");
      subjects.store(out);
      routes.store(out);
      Functions.storeConfigFooter(out, "routes");
      out.flush();
      out.close();
      Functions.renameNewSavedFileToOriginal(newFile, file);
    }
    catch (FileNotFoundException e)
    {
      throw new AdminException("Couldn't save new routes settings: Couldn't write to destination config filename: " + e.getMessage());
    } catch (IOException e) {
      throw new AdminException("Couldn't save new routes settings: " + e.getMessage());
    }
  }

  synchronized public void save() throws AdminException {
    saveToFile(SMSC_ROUTES_TEMPORAL_CONFIG);
  }
  synchronized public void restore() throws AdminException {
    loadFromFile(SMSC_ROUTES_TEMPORAL_CONFIG);
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
