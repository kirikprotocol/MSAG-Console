/*
 * Created by igork
 * Date: 24.09.2002
 * Time: 17:57:03
 */
package ru.novosoft.smsc.jsp.smsc.services;

import org.xml.sax.SAXException;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.SME;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.xml.Utils;
import ru.novosoft.util.jsp.MultipartDataSource;
import ru.novosoft.util.jsp.MultipartServletRequest;

import javax.xml.parsers.FactoryConfigurationError;
import javax.xml.parsers.ParserConfigurationException;
import java.io.*;
import java.util.List;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

public class ServiceAddExternalAdm extends SmeBean
{
	private final static String SYSTEM_ID_PARAM_NAME = "system id";

	private byte stage = 0;
	private String hostName = null;
	private int port = -1;
	private String startupArgs = null;

	private String mbNext = null;
	private String mbCancel = null;

	private File incomingZip = null;


	public int process(SMSCAppContext appContext, List errors, java.security.Principal loginedPrincipal)
	{
		int result = super.process(appContext, errors, loginedPrincipal);
		if (result != RESULT_OK)
			return result;

		if (mbCancel != null)
		{
			mbCancel = null;
			cleanup();
			return RESULT_DONE;
		}
		else if (mbNext != null)
		{
			mbNext = null;
			switch (stage)
			{
				case 0:
					stage++;
					return RESULT_OK;
				case 1:
					return RESULT_OK;
				case 2:
					return processStage2();
				case 3:
					return processStage3();
				default:
					return RESULT_DONE;
			}
		}
		else
			return RESULT_OK;
	}

	protected void cleanup()
	{
		if (incomingZip != null && incomingZip.isFile() && incomingZip.exists())
			incomingZip.delete();

		hostName = null;
		port = -1;
		startupArgs = null;
		mbNext = null;
		mbCancel = null;
		super.cleanup();
	}

	/**
	 * MUST be called on stage 1
	 */
	public int receiveFile(MultipartServletRequest multi)
	{
		if (multi == null)
			return error("File not attached");

		MultipartDataSource dataFile = null;
		try
		{
			dataFile = multi.getMultipartDataSource("distribute");
			if (dataFile == null)
				return error("Service distributive not attached");
			if (dataFile.getContentType().equals("application/x-zip-compressed"))
			{
				incomingZip = Functions.saveFileToTemp(dataFile.getInputStream(), "SMSC_SME_distrib_", ".zip.tmp");
				dataFile.close();
				dataFile = null;

				checkServiceContent(incomingZip);
				serviceId = extractSystemId(incomingZip);
				if (hostsManager.getSmeIds().contains(serviceId))
				{
					incomingZip.delete();
					incomingZip = null;
					return error("Service \"" + serviceId + "\" already exists");
				}
			}
			else
				return error("Distributive file must be zip compressed");
		}
		catch (Throwable t)
		{
			return error("Couldn't receive file", t);
		}
		finally
		{
			if (dataFile != null)
			{
				dataFile.close();
				dataFile = null;
			}
		}
		if (errors.size() == 0)
		{
			stage++;
			return RESULT_OK;
		}
		else
		{
			return RESULT_ERROR;
		}
	}

	private int processStage2()
	{
		if (hostName == null || hostName.length() < 1)
			error("Host not selected");
		if (port < 0 || port > 0xFFFF)
			error("Incorrect port value");
		if (startupArgs == null)
			startupArgs = "";

		if (errors.size() == 0)
		{
			stage++;
			wantAlias = true;
			forceDC = true;
			return RESULT_OK;
		}
		else
			return RESULT_ERROR;
	}

	private int processStage3()
	{
		if (hostsManager.getSmeIds().contains(serviceId))
			return error(SMSCErrors.error.services.alreadyExists, serviceId);
		if (priority < 0 || priority > MAX_PRIORITY)
			return error(SMSCErrors.error.services.invalidPriority, String.valueOf(priority));


		ServiceInfo serviceInfo = null;
		try
		{
			serviceInfo = new ServiceInfo(serviceId, hostName, port, startupArgs, new SME(serviceId, priority, SME.SMPP, typeOfNumber, numberingPlan, convertInterfaceVersion(interfaceVersion), systemType, "", rangeOfAddress, -1, wantAlias, forceDC, timeout, receiptSchemeName, disabled, mode), ServiceInfo.STATUS_STOPPED);
		}
		catch (NullPointerException e)
		{
			return error(SMSCErrors.error.services.NotAllParametersDefined);
		}

		try
		{
			hostsManager.deployAdministrableService(incomingZip, serviceInfo);
			appContext.getStatuses().setServicesChanged(true);
		}
		catch (AdminException e)
		{
			logger.error("Adding service \"" + serviceInfo.getId() + "\" to host \"" + serviceInfo.getHost() + "\" failed", e);
			return error("Adding service \"" + serviceInfo.getId() + "\" to host \"" + serviceInfo.getHost() + "\" failed: " + e.getMessage());
		}
		logger.info("New service \"" + serviceInfo.getId() + "\" added to host \"" + serviceInfo.getHost() + '"');
		stage = 0;
		cleanup();
		return RESULT_DONE;
	}

	public List getHostNames()
	{
		return hostsManager.getHostNames();
	}

	/************************************** ***********************************/

	private void checkServiceContent(File incomingZip) throws AdminException
	{
		boolean serviceFound = false;
		boolean confFound = false;
		boolean jspFound = false;
		boolean configFound = false;

		try
		{
			ZipInputStream zin = new ZipInputStream(new BufferedInputStream(new FileInputStream(incomingZip)));
			ZipEntry entry = zin.getNextEntry();

			while (entry != null && !(serviceFound && confFound && jspFound && configFound))
			{
				if (!serviceFound && entry.getName().equals("bin/services"))
					serviceFound = true;
				if (!confFound && (entry.getName().startsWith("conf/") || (entry.getName().equals("conf") && entry.isDirectory())))
					confFound = true;
				if (!jspFound && (entry.getName().startsWith("jsp/") || (entry.getName().equals("jsp") && entry.isDirectory())))
					jspFound = true;
				if (!configFound && entry.getName().equals("config.xml"))
					configFound = true;

				entry = zin.getNextEntry();
			}
			zin.close();
		}
		catch (IOException e)
		{
			logger.error("Couldn't check incoming services archive integrity", e);
			throw new AdminException("Couldn't check incoming services archive integrity, nested: " + e.getMessage());
		}
	}

	private String extractSystemId(File incomingZip) throws AdminException
	{
		try
		{
			ZipInputStream zin = new ZipInputStream(new BufferedInputStream(new FileInputStream(incomingZip)));
			ZipEntry entry = zin.getNextEntry();

			while (entry != null && !entry.getName().equals("config.xml"))
				entry = zin.getNextEntry();

			if (entry == null)
				throw new AdminException("/config.xml not found in distributive");

			Config serviceConfig = new Config(Utils.parse(new InputStreamReader(zin)));
			return serviceConfig.getString(SYSTEM_ID_PARAM_NAME);
		}
		catch (IOException e)
		{
			logger.error("Couldn't read or unzip incoming archive", e);
			throw new AdminException("Couldn't read or unzip incoming archive, nested: " + e.getMessage());
		}
		catch (FactoryConfigurationError error)
		{
			logger.error("Couldn't parse incoming services configuration", error);
			throw new AdminException("Couldn't parse incoming services configuration" + error.getMessage());
		}
		catch (ParserConfigurationException e)
		{
			logger.error("Couldn't parse incoming services configuration", e);
			throw new AdminException("Couldn't parse incoming services configuration" + e.getMessage());
		}
		catch (SAXException e)
		{
			logger.error("Couldn't parse incoming services configuration", e);
			throw new AdminException("Couldn't parse incoming services configuration, nested: " + e.getMessage());
		}
		catch (NullPointerException e)
		{
			logger.error("Couldn't get incoming services id", e);
			throw new AdminException("Couldn't get incoming services id, nested: " + e.getMessage());
		}
		catch (Config.ParamNotFoundException e)
		{
			logger.error("Couldn't get incoming services id", e);
			throw new AdminException("Couldn't get incoming services id, nested: " + e.getMessage());
		}
		catch (Config.WrongParamTypeException e)
		{
			logger.error("Couldn't get incoming services id", e);
			throw new AdminException("Couldn't get incoming services id, nested: " + e.getMessage());
		}
	}


	/*********************************** Properties **********************************/

	public byte getStage()
	{
		return stage;
	}

	public void setStage(byte stage)
	{
		this.stage = stage;
	}

	public String getHostName()
	{
		return hostName;
	}

	public void setHostName(String hostName)
	{
		this.hostName = hostName;
	}

	public String getPort()
	{
		return Integer.toString(port);
	}

	public void setPort(String port)
	{
		try
		{
			this.port = Integer.decode(port).intValue();
		}
		catch (NumberFormatException e)
		{
			this.port = 0;
		}
	}

	public String getStartupArgs()
	{
		return startupArgs;
	}

	public void setStartupArgs(String startupArgs)
	{
		this.startupArgs = startupArgs;
	}

	public String getMbNext()
	{
		return mbNext;
	}

	public void setMbNext(String mbNext)
	{
		this.mbNext = mbNext;
	}

	public String getMbCancel()
	{
		return mbCancel;
	}

	public void setMbCancel(String mbCancel)
	{
		this.mbCancel = mbCancel;
	}
}
