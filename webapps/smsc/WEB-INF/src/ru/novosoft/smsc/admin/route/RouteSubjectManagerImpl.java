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

  private final static String SMSC_ROUTES_PRIMARY_CONFIG  = "routes.xml";
  private final static String SMSC_ROUTES_TEMPORAL_CONFIG = "routes_.xml";

	public RouteSubjectManagerImpl(SmeManager smeManager) throws AdminException
	{
    this.smeManager = smeManager;
    load();
	}

	public RouteList getRoutes()
	{
		return routes;
	}

	public SubjectList getSubjects()
	{
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

  private void copyFile(File source, File destination)
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
  }

  synchronized public void load() throws AdminException
  {
    try
    {
      final File smscConfFolder = WebAppFolders.getSmscConfFolder();
      File primaryConfig = new File(smscConfFolder, SMSC_ROUTES_PRIMARY_CONFIG);
      File temporalConfig = new File(smscConfFolder, SMSC_ROUTES_TEMPORAL_CONFIG);

      Document routesDoc = Utils.parse(new FileReader(primaryConfig));
      subjects = new SubjectList(routesDoc.getDocumentElement(), smeManager);
      routes = new RouteList(routesDoc.getDocumentElement(), subjects, smeManager);

      copyFile(primaryConfig, temporalConfig);
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

  synchronized public void save() throws AdminException
  {
    try
    {
      PrintWriter out = new PrintWriter(new FileOutputStream(
          new File(WebAppFolders.getSmscConfFolder(), SMSC_ROUTES_TEMPORAL_CONFIG)), true);
      Functions.storeConfigHeader(out, "routes", "routes.dtd");
      subjects.store(out);
      routes.store(out);
      Functions.storeConfigFooter(out, "routes");
      out.close();
    }
    catch (FileNotFoundException e)
    {
      throw new AdminException("Couldn't save new routes settings: Couldn't write to destination config file: " + e.getMessage());
    }
  }

  synchronized public void apply() throws AdminException
	{
		save(); // save routes info into temporal file, than copy temporal to primary

    try
    {
      final File smscConfFolder = WebAppFolders.getSmscConfFolder();
      File primaryConfig = new File(smscConfFolder, SMSC_ROUTES_PRIMARY_CONFIG);
      File temporalConfig = new File(smscConfFolder, SMSC_ROUTES_TEMPORAL_CONFIG);

      copyFile(temporalConfig, primaryConfig);
    }
    catch (IOException e)
    {
      logger.error("Couldn't perform IO operation", e);
      throw new AdminException("Couldn't perform IO operation: " + e.getMessage());
    }
	}
}
