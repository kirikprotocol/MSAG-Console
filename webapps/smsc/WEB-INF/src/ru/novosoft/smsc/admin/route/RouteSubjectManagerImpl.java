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

	public RouteSubjectManagerImpl(SmeManager smeManager) throws AdminException
	{
		try
		{
			final File smscConfFolder = WebAppFolders.getSmscConfFolder();
			Document routesDoc = Utils.parse(new FileReader(new File(smscConfFolder, "routes.xml")));
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
			logger.error("Couldn't read", e);
			throw new AdminException("Couldn't read: " + e.getMessage());
		}
		catch (NullPointerException e)
		{
			logger.error("Couldn't parse", e);
			throw new AdminException("Couldn't parse: " + e.getMessage());
		}
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

	public void save() throws AdminException
	{
		try
		{
			PrintWriter out = new PrintWriter(new FileOutputStream(new File(WebAppFolders.getSmscConfFolder(), "routes.xml")), true);
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
}
