/*
 * Created by igork
 * Date: 24.09.2002
 * Time: 17:57:03
 */
package ru.novosoft.smsc.jsp.smsc.services;

import org.apache.log4j.Category;
import org.xml.sax.SAXException;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.daemon.Daemon;
import ru.novosoft.smsc.admin.route.SME;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.xml.Utils;
import ru.novosoft.util.jsp.MultipartDataSource;
import ru.novosoft.util.jsp.MultipartServletRequest;

import javax.xml.parsers.FactoryConfigurationError;
import javax.xml.parsers.ParserConfigurationException;
import java.io.*;
import java.util.List;
import java.util.Set;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

public class ServiceAddExternalAdm extends PageBean
{
	protected final static String SYSTEM_ID_PARAM_NAME = "system id";

	protected Category logger = Category.getInstance(this.getClass());

	protected byte stage = 0;
	private String serviceId = null;
	private int priority = 0;
	private String hostName = null;
	private int port = -1;
	private String startupArgs = null;
	private String systemType = null;
	private int typeOfNumber = -1;
	private int numberingPlan = -1;
	private int interfaceVersion = -1;
	private String rangeOfAddress = null;
	private String password = null;
	private boolean wantAlias = false;
	private int timeout = 8;

	protected String mbNext = null;
	protected String mbCancel = null;

	private File incomingZip = null;


	public int process(SMSCAppContext appContext, List errors)
	{
		int result = super.process(appContext, errors);
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
				java.io.InputStream is = dataFile.getInputStream();
				incomingZip = saveFileToTemp(is);
				dataFile.close();
				dataFile = null;

				checkServiceContent(incomingZip);
				serviceId = extractSystemId(incomingZip);
				if (serviceManager.getSmeIds().contains(serviceId))
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

	protected int processStage2()
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
			return RESULT_OK;
		}
		else
			return RESULT_ERROR;
	}

	protected int processStage3()
	{
		if (serviceManager.getSmeIds().contains(serviceId))
			return error(SMSCErrors.error.services.alreadyExists, serviceId);
		if (priority < 0 || priority > MAX_PRIORITY)
			return error(SMSCErrors.error.services.invalidPriority, String.valueOf(priority));


		ServiceInfo serviceInfo = null;
		try
		{
			serviceInfo = new ServiceInfo(serviceId,
													hostName,
													port,
													startupArgs,
													new SME(serviceId,
															  priority,
															  SME.SMPP,
															  typeOfNumber,
															  numberingPlan,
															  interfaceVersion,
															  systemType,
															  "",
															  rangeOfAddress,
															  -1,
															  wantAlias,
															  timeout),
													ServiceInfo.STATUS_STOPPED);
		}
		catch (NullPointerException e)
		{
			return error(SMSCErrors.error.services.NotAllParametersDefined);
		}

		try
		{
			serviceManager.deployAdministrableService(incomingZip, serviceInfo);
			Daemon d = daemonManager.getDaemon(serviceInfo.getHost());
			d.addService(serviceInfo);
			appContext.getStatuses().setServicesChanged(true);
		}
		catch (AdminException e)
		{
			logger.error("Adding service \"" + serviceInfo.getId() + "\" to host \"" + serviceInfo.getHost() + "\" failed", e);
			return error("Adding service \"" + serviceInfo.getId() + "\" to host \"" + serviceInfo.getHost() + "\" failed: " + e.getMessage());
		}
		logger.info("New service \"" + serviceInfo.getId() + "\" added to host \"" + serviceInfo.getHost() + '"');
		stage = 0;
		return RESULT_DONE;
	}

	public Set getHostNames()
	{
		return daemonManager.getHosts();
	}

	/************************************** ***********************************/

	protected void checkServiceContent(File incomingZip)
			throws AdminException
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

	protected File saveFileToTemp(InputStream in)
			throws AdminException
	{
		File tmpFile = null;

		try
		{
			tmpFile = File.createTempFile("SMSC_SME_distrib_", ".zip.tmp");
			OutputStream out = new BufferedOutputStream(new FileOutputStream(tmpFile));

			byte buffer[] = new byte[2048];
			for (int readed = 0; (readed = in.read(buffer)) > -1;)
			{
				out.write(buffer, 0, readed);
			}

			in.close();
			out.close();
		}
		catch (IOException e)
		{
			logger.error("Couldn't save incoming services archive to temporary file", e);
			throw new AdminException("Couldn't save incoming services archive to temporary file, nested: " + e.getMessage());
		}
		return tmpFile;
	}

	protected String extractSystemId(File incomingZip)
			throws AdminException
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

	public String getServiceId()
	{
		return serviceId;
	}

	public String getHostName()
	{
		return hostName;
	}

	public void setHostName(String hostName)
	{
		this.hostName = hostName;
	}

	public int getPort()
	{
		return port;
	}

	public void setPort(int port)
	{
		this.port = port;
	}

	public String getStartupArgs()
	{
		return startupArgs;
	}

	public void setStartupArgs(String startupArgs)
	{
		this.startupArgs = startupArgs;
	}

	public String getSystemType()
	{
		return systemType;
	}

	public void setSystemType(String systemType)
	{
		this.systemType = systemType;
	}

	public int getTypeOfNumber()
	{
		return typeOfNumber;
	}

	public void setTypeOfNumber(int typeOfNumber)
	{
		this.typeOfNumber = typeOfNumber;
	}

	public int getNumberingPlan()
	{
		return numberingPlan;
	}

	public void setNumberingPlan(int numberingPlan)
	{
		this.numberingPlan = numberingPlan;
	}

	public String getInterfaceVersion()
	{
		return "" + ((interfaceVersion >> 4) & 0x0F) + '.' + (interfaceVersion & 0x0F);
	}

	public void setInterfaceVersion(String newInterfaceVersion)
	{
		int pos = newInterfaceVersion.indexOf('.');
		if (pos > 0)
		{
			this.interfaceVersion = (Integer.parseInt(newInterfaceVersion.substring(0, pos)) << 4) + Integer.parseInt(newInterfaceVersion.substring(pos + 1));
		}
		else
			this.interfaceVersion = 0x34;
	}

	public String getRangeOfAddress()
	{
		return rangeOfAddress;
	}

	public void setRangeOfAddress(String rangeOfAddress)
	{
		this.rangeOfAddress = rangeOfAddress;
	}

	public String getPassword()
	{
		return password;
	}

	public void setPassword(String password)
	{
		this.password = password;
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

	public int getPriority()
	{
		return priority;
	}

	public void setPriority(int priority)
	{
		this.priority = priority;
	}

	public boolean isWantAlias()
	{
		return wantAlias;
	}

	public void setWantAlias(boolean wantAlias)
	{
		this.wantAlias = wantAlias;
	}

	public int getTimeout()
	{
		return timeout;
	}

	public void setTimeout(int timeout)
	{
		this.timeout = timeout;
	}
}
