package ru.novosoft.smsc.admin.smsc_service;

import org.apache.log4j.Category;
import org.w3c.dom.Document;
import org.xml.sax.SAXException;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.SME;
import ru.novosoft.smsc.admin.route.SMEList;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.util.*;
import ru.novosoft.smsc.util.xml.Utils;

import javax.xml.parsers.FactoryConfigurationError;
import javax.xml.parsers.ParserConfigurationException;
import java.io.*;
import java.util.*;

/**
 * Created by igork
 * Date: Jan 30, 2003
 * Time: 3:19:25 PM
 */
public class SmeManagerImpl implements SmeManager
{
	private Category logger = Category.getInstance(this.getClass());
	private SMEList smes = null;
	private File smeConfigFile = null;
	private Smsc smsc;

	public SmeManagerImpl(Smsc smsc) throws AdminException
	{
		this.smsc = smsc;
		try
		{
			smeConfigFile = new File(WebAppFolders.getSmscConfFolder(), "sme.xml");
			Document smesDoc = Utils.parse(new FileReader(smeConfigFile));
			smes = new SMEList(smesDoc.getDocumentElement());
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
			logger.error("Couldn't read sme config", e);
			throw new AdminException("Couldn't read sme config: " + e.getMessage());
		}
		catch (NullPointerException e)
		{
			logger.error("Couldn't parse sme config", e);
			throw new AdminException("Couldn't parse sme config: " + e.getMessage());
		}
	}

	public synchronized void removeAllIfSme(Collection serviceIds) throws AdminException
	{
		for (Iterator i = serviceIds.iterator(); i.hasNext();)
		{
			String serviceId = (String) i.next();
			if (contains(serviceId))
			{
				remove(serviceId);
			}
		}
	}

	public synchronized SME remove(String id) throws AdminException
	{
		SME removedSme = smes.remove(id);
		save();
		if (smsc.getInfo().getStatus() == ServiceInfo.STATUS_RUNNING)
			smsc.smeRemove(id);
		return removedSme;
	}

	public synchronized boolean contains(String id)
	{
		return smes.contains(id);
	}

	public synchronized SME get(String id) throws AdminException
	{
		return smes.get(id);
	}

	public synchronized SME add(String id, int priority, byte type, int typeOfNumber, int numberingPlan, int interfaceVersion, String systemType, String password, String addrRange, int smeN, boolean wantAlias, boolean forceDC, int timeout, String receiptSchemeName, boolean disabled, byte mode) throws AdminException
	{
		return add(new SME(id, priority, type, typeOfNumber, numberingPlan, interfaceVersion, systemType, password, addrRange, smeN, wantAlias, forceDC, timeout, receiptSchemeName, disabled, mode));
	}

	public synchronized SME add(SME newSme) throws AdminException
	{
		smes.add(newSme);
		save();
		if (smsc.getInfo().getStatus() == ServiceInfo.STATUS_RUNNING)
			smsc.smeAdd(newSme);
		return newSme;
	}

	public synchronized List getSmeNames()
	{
		return new SortedList(smes.getNames());
	}

/*
	public SMEList getSmes()
	{
		return smes;
	}
*/

	public synchronized void save() throws AdminException
	{
		try
		{
			PrintWriter out = new PrintWriter(new FileOutputStream(smeConfigFile), true);
			Functions.storeConfigHeader(out, "records", "SmeRecords.dtd");
			smes.store(out);
			Functions.storeConfigFooter(out, "records");
			out.flush();
			out.close();
		}
		catch (FileNotFoundException e)
		{
			throw new AdminException("Couldn't save new smes settings: Couldn't write to destination config file: " + e.getMessage());
		}
	}

	public synchronized SME update(SME newSme) throws AdminException
	{
		SME updatedSme = smes.update(newSme);
		save();
		if (smsc.getInfo().getStatus() == ServiceInfo.STATUS_RUNNING)
			smsc.smeUpdate(updatedSme);
		return updatedSme;
	}
}
