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
	private Smsc smsc = null;

	public SmscList(final Config webappConfig, final NSConnectionPool connectionPool, SMSCAppContext smscAppContext) throws AdminException
	{
		this.config = webappConfig;
		this.appContext = smscAppContext;
		Set nodeNames = config.getSectionChildSectionNames("smscenters");
		logger.debug("Initializing SMSC list ");

		try
		{
			Map stopFileNames = new HashMap();
			for (Iterator i = nodeNames.iterator(); i.hasNext();)
			{
				String encodedName = (String) i.next();
				String nodeName = StringEncoderDecoder.decodeDot(encodedName.substring(encodedName.lastIndexOf('.') + 1));
				final String stopFileName = config.getString(encodedName + ".stopfilename");
				stopFileNames.put(nodeName, stopFileName);
			}
		String smscConfFolder = webappConfig.getString("smsc.config folder");
		WebAppFolders.setSmscConfFolder(new File(smscConfFolder));
		smsc = new Smsc(Constants.SMSC_SME_ID, webappConfig.getString("smsc.host"), webappConfig.getInt("smsc.port"), smscConfFolder, stopFileNames, connectionPool, appContext);
		logger.debug("SMSC added");
		}
		catch (AdminException e)
		{
			logger.error("Couldn't add SMSC \"", e);
			throw e;
		}
		catch (Config.ParamNotFoundException e)
		{
			logger.debug("Misconfigured SMSC \"", e);
			throw new AdminException("Miscofigured SMSC");
		}
		catch (Config.WrongParamTypeException e)
		{
			logger.debug("Misconfigured SMSC \"", e);
			throw new AdminException("Miscofigured SMSC");
		}
		logger.debug("SMSC list initialized");
		appContext.setSmsc(smsc);
	}

	public void updateSmsc()
	{
		appContext.setSmsc(getSmsc());
	}

	public Smsc getSmsc()
	{
		return smsc;
	}

	public void switchSmsc(String nodeName) throws AdminException
	{
		appContext.getHostsManager().getService(Constants.SMSC_SME_ID).switchOver(nodeName);
		updateSmsc();
	}

	public void applyConfig() throws AdminException
	{
		try
		{
			getSmsc().getSmscConfig().save();
		}
		catch(Exception e)
		{
			e.printStackTrace();
			throw new AdminException("Couldn't save Smsc config");
		}
	}

	public String[] getNodes() throws AdminException
	{
/*		String[] result = new String[getSmsc().getStopFileName().keySet().size()];
		int n = 0;
		for (Iterator i = getSmsc().getStopFileName().keySet().iterator(); i.hasNext();)
		{
			result[n] = (String) i.next();
			n++;
		}
		return result;*/
		return appContext.getHostsManager().getServiceNodes(Constants.SMSC_SME_ID);
	}

	public ResourceGroup getSmscResourceGroup() throws AdminException
	{
		return appContext.getHostsManager().getService(Constants.SMSC_SME_ID);
	}
}