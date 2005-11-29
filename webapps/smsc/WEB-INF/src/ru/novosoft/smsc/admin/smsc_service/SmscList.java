package ru.novosoft.smsc.admin.smsc_service;

import java.util.*;
import java.io.File;

import ru.novosoft.smsc.admin.smsc_service.Smsc;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.resource_group.ResourceGroup;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.service.Service;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.WebAppFolders;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.util.conpool.NSConnectionPool;
import org.apache.log4j.Category;

/**
 * Created by IntelliJ IDEA.
 * User: starkom
 * Date: 07.07.2005
 * Time: 14:05:08
 * To change this template use File | Settings | File Templates.
 */

public class SmscList
{
	private Config config = null;
	private SMSCAppContext appContext = null;
	private Category logger = Category.getInstance(this.getClass());
	private Map smscenters = new HashMap();

	public SmscList(final Config webappConfig, final NSConnectionPool connectionPool, SMSCAppContext smscAppContext) throws AdminException
	{
		this.config = webappConfig;
		this.appContext = smscAppContext;
		Set smscNames = config.getSectionChildSectionNames("smscenters");
		logger.debug("Initializing SMSC list ");
		for (Iterator i = smscNames.iterator(); i.hasNext();)
		{
			String encodedName = (String) i.next();
			String smscName = StringEncoderDecoder.decodeDot(encodedName.substring(encodedName.lastIndexOf('.') + 1));
			try
			{
				final int port = config.getInt(encodedName + ".port");
				final String host = config.getString(encodedName + ".host");
				final String configFolder = config.getString(encodedName + ".config folder");
				WebAppFolders.setSmscConfFolder(new File(configFolder));
				final String stopfilename = config.getString(encodedName + ".stopfilename");
				Smsc smsc = new Smsc(smscName, host, port, configFolder, stopfilename, connectionPool, smscAppContext);
				smscenters.put(smscName, smsc);
				appContext.getServiceManager().add(smsc);
				logger.debug("SMSC \"" + smscName + "\"" + host + ":" + port + "added");
			}
			catch (AdminException e)
			{
				logger.error("Couldn't add SMSC \"" + smscName + "\"", e);
				throw e;
			}
			catch (Config.ParamNotFoundException e)
			{
				logger.debug("Misconfigured SMSC \"" + smscName + "\"", e);
				throw new AdminException("Miscofigured SMSC");
			}
			catch (Config.WrongParamTypeException e)
			{
				logger.debug("Misconfigured SMSC \"" + smscName + "\"", e);
				throw new AdminException("Miscofigured SMSC");
			}
		}
		logger.debug("SMSC list initialized");
		if (smscenters.isEmpty()) {throw new AdminException("There is no one SMSC");}
		appContext.setSmsc((Smsc)smscenters.values().toArray()[0]);
	}

	public void updateSmsc()
	{
		appContext.setSmsc(getSmsc());
	}

	public Smsc getSmsc()
	{
		try
		{
			ResourceGroup smscResGroup = appContext.getHostsManager().getService(Constants.SMSC_SME_ID);
			String[] nodes = smscResGroup.listNodes();
			for (int i = 0; i < nodes.length; i++)
			{
				if (smscResGroup.getOnlineStatus(nodes[i]) == ServiceInfo.STATUS_ONLINE)
					return (Smsc)smscenters.get(nodes[i]);
			}
		}
		catch (AdminException e) {return (Smsc)smscenters.values().toArray()[0];}
		return (Smsc)smscenters.values().toArray()[0];
	}

	public void switchSmsc(String nodeName) throws AdminException
	{
		appContext.getHostsManager().getService(Constants.SMSC_SME_ID).switchOver(nodeName);
		updateSmsc();
	}

	public Map getSmsCenters()
	{
		return smscenters;
	}

	public Smsc getSmsc(String SmscName)
	{
		Smsc result = null;
/*		if (mainSmsc.getInfo().getId().equals(SmscName)) {result = mainSmsc;}
		else if (secondarySmsc.getInfo().getId().equals(SmscName)) result = secondarySmsc;*/
		return result;
	}

	public void applyConfig(String nodeName) throws AdminException
	{
		getSmsc(nodeName).applyConfig();
/*		if (secondarySmsc == null) {mainSmsc.applyConfig();return;} //если нет второго, то просто апплай

		if (secondarySmsc.getInfo().getStatus() != ServiceInfo.STATUS_RUNNING) //если второй не запущен - запустить
		{
			appContext.getHostsManager().startService(secondarySmsc.getInfo().getId());
			while ((secondarySmsc.getInfo().getStatus() != ServiceInfo.STATUS_RUNNING)&&(secondarySmsc.getInfo().getStatus() != ServiceInfo.STATUS_UNKNOWN))
				try
				{
					Thread.sleep(50);
				}
				catch (InterruptedException e) {e.printStackTrace();}
		}

		Smsc forApply = mainSmsc;
		switchSmsc();

		forApply.applyConfig();

		while ((forApply.getInfo().getStatus() != ServiceInfo.STATUS_RUNNING)&&(forApply.getInfo().getStatus() != ServiceInfo.STATUS_UNKNOWN))
			try
			{
				Thread.sleep(50);
			}
			catch (InterruptedException e) {e.printStackTrace();}

		switchSmsc();*/
	}

	public String[] getNodes() throws AdminException
	{
		return appContext.getHostsManager().getServiceNodes(Constants.SMSC_SME_ID);
	}
}