/*
 * Created by igork
 * Date: 24.09.2002
 * Time: 17:57:03
 */
package ru.novosoft.smsc.admin.service;

import org.apache.log4j.Category;
import org.xml.sax.SAXException;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.SME;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.xml.Utils;

import javax.xml.parsers.FactoryConfigurationError;
import javax.xml.parsers.ParserConfigurationException;
import java.io.*;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

public class AddAdmServiceWizard
{
	protected final static String SYSTEM_ID_PARAM_NAME = "system id";

	protected Category logger = Category.getInstance(this.getClass());

	private File incomingZip = null;
	private String systemId = null;
	private String host = null;
	private int port = -1;
	private String startupArgs = null;
	private String systemType = null;
	private int typeOfNumber = -1;
	private int numberingPlan = -1;
	private int interfaceVersion = -1;
	private String rangeOfAddress = null;


	public AddAdmServiceWizard(InputStream inZip)
			  throws AdminException
	{
		this.incomingZip = saveFileToTemp(inZip);
		checkServiceContent(incomingZip);
		systemId = extractSystemId(incomingZip);
	}

	public void setStage2(String host, int port, String startupArgs)
	{
		this.host = host;
		this.port = port;
		this.startupArgs = startupArgs;
	}

	public boolean checkStage2()
	{
		return host != null && host.length() > 0 && port > 0 && startupArgs != null;
	}

	public void setStage3(String systemType, int typeOfNumber, int numberingPlan, int interfaceVersion,
								 String rangeOfAddress)
	{
		this.systemType = systemType;
		this.typeOfNumber = typeOfNumber;
		this.numberingPlan = numberingPlan;
		this.interfaceVersion = interfaceVersion;
		this.rangeOfAddress = rangeOfAddress;
	}

	public boolean checkStage3()
	{
		return systemType != null && systemType.length() > 0
				  && typeOfNumber >= 0 && numberingPlan >= 0 && interfaceVersion >= 0
				  && rangeOfAddress != null && rangeOfAddress.length() > 0;
	}

	public String getSystemId()
	{
		return systemId;
	}

	public String getHost()
	{
		return host;
	}

	public int getPort()
	{
		return port;
	}

	public String getStartupArgs()
	{
		return startupArgs;
	}

	public String getSystemType()
	{
		return systemType;
	}

	public int getTypeOfNumber()
	{
		return typeOfNumber;
	}

	public int getNumberingPlan()
	{
		return numberingPlan;
	}

	public int getInterfaceVersion()
	{
		return interfaceVersion;
	}

	public String getRangeOfAddress()
	{
		return rangeOfAddress;
	}

	public void deploy(File daemonsFolder, File webappFolder, File webinfLibFolder)
			  throws AdminException
	{

		try
		{
			File daemonFolder = new File(daemonsFolder, host);
			File serviceFolder = new File(daemonFolder, systemId);
			File jspsFolder = ServiceManager.getServiceJspsFolder(webappFolder, systemId);

			if (serviceFolder.exists())
				throw new AdminException("Service already exists in filesystem");
			if (jspsFolder.exists())
				throw new AdminException("Jps pages for new service already exists");

			unZipArchive(serviceFolder,
							 new BufferedInputStream(new FileInputStream(incomingZip)));

			File incomingJsps = new File(serviceFolder, "jsp");
			if (!incomingJsps.renameTo(jspsFolder))
				throw new AdminException("Couldn't deploy JSP's (\"" + incomingJsps.getCanonicalPath() + "\") to \"" + jspsFolder.getCanonicalPath() + "\"");

			File newLogFolder = new File(serviceFolder, "log");
			newLogFolder.mkdir();
			moveJars(new File(serviceFolder, "lib"), webinfLibFolder);
		}
		catch (AdminException e)
		{
			rollbackDeploy(daemonsFolder, webappFolder, webinfLibFolder);
			throw e;
		}
		catch (IOException e)
		{
			rollbackDeploy(daemonsFolder, webappFolder, webinfLibFolder);
			throw new AdminException("Couldnt deploy new service, nested: " + e.getMessage());
		}
	}

	public SME createSme()
			  throws AdminException
	{
		checkStage2();
		checkStage3();
		return new SME(systemId, SME.SMPP, typeOfNumber, numberingPlan, interfaceVersion, systemType, "", rangeOfAddress, -1);
	}

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
				if (!serviceFound && entry.getName().equals("bin/service"))
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
			logger.error("Couldn't check incoming service archive integrity", e);
			throw new AdminException("Couldn't check incoming service archive integrity, nested: " + e.getMessage());
		}
	}

	protected void rollbackDeploy(File daemonsFolder, File webappFolder, File webinfLibFolder)
	{
		File daemonFolder = new File(daemonsFolder, host);
		File serviceFolder = new File(daemonFolder, systemId);
		File jspsFolder = ServiceManager.getServiceJspsFolder(webappFolder, systemId);

		deleteFolder(serviceFolder);
		deleteFolder(jspsFolder);
	}

	protected void deleteFolder(File folder)
	{
		if (folder.exists())
		{
			File files[] = folder.listFiles();
			for (int i = 0; i < files.length; i++)
			{
				if (files[i].isDirectory())
					deleteFolder(files[i]);
				else
					files[i].delete();
			}
			folder.delete();
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
			logger.error("Couldn't save incoming service archive to temporary file", e);
			throw new AdminException("Couldn't save incoming service archive to temporary file, nested: " + e.getMessage());
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

			Config serviceConfig = new Config(Utils.parse(zin));
			return serviceConfig.getString(SYSTEM_ID_PARAM_NAME);
		}
		catch (IOException e)
		{
			logger.error("Couldn't read or unzip incoming archive", e);
			throw new AdminException("Couldn't read or unzip incoming archive, nested: " + e.getMessage());
		}
		catch (FactoryConfigurationError error)
		{
			logger.error("Couldn't parse incoming service configuration", error);
			throw new AdminException("Couldn't parse incoming service configuration" + error.getMessage());
		}
		catch (ParserConfigurationException e)
		{
			logger.error("Couldn't parse incoming service configuration", e);
			throw new AdminException("Couldn't parse incoming service configuration" + e.getMessage());
		}
		catch (SAXException e)
		{
			logger.error("Couldn't parse incoming service configuration", e);
			throw new AdminException("Couldn't parse incoming service configuration, nested: " + e.getMessage());
		}
		catch (NullPointerException e)
		{
			logger.error("Couldn't get incoming service id", e);
			throw new AdminException("Couldn't get incoming service id, nested: " + e.getMessage());
		}
		catch (Config.ParamNotFoundException e)
		{
			logger.error("Couldn't get incoming service id", e);
			throw new AdminException("Couldn't get incoming service id, nested: " + e.getMessage());
		}
		catch (Config.WrongParamTypeException e)
		{
			logger.error("Couldn't get incoming service id", e);
			throw new AdminException("Couldn't get incoming service id, nested: " + e.getMessage());
		}
	}

	protected void unZipFileFromArchive(File folderUnpackTo, String name, ZipInputStream zin)
			  throws IOException
	{
		File file = new File(folderUnpackTo, name);
		file.getParentFile().mkdirs();
		OutputStream out = new BufferedOutputStream(new FileOutputStream(file));
		for (int i = 0; (i = zin.read()) != -1; out.write(i)) ;
		out.close();
	}

	protected void unZipArchive(File folderUnpackTo, InputStream in)
			  throws IOException
	{
		ZipInputStream zin = new ZipInputStream(in);
		for (ZipEntry e = zin.getNextEntry(); e != null; e = zin.getNextEntry())
		{
			unZipFileFromArchive(folderUnpackTo, e.getName(), zin);
		}
		zin.close();
		in.close();
	}

	protected void moveJars(File serviceFolder, File jarsFolder)
	{
		File[] jars = serviceFolder.listFiles();
		if (jars != null)
		{
			for (int i = 0; i < jars.length; i++)
			{
				if (jars[i].isFile() && jars[i].getName().endsWith(".jar"))
				{
					File newName = new File(jarsFolder, jars[i].getName());
					try
					{
						if (!jars[i].renameTo(newName))
							logger.error("couldn't rename \"" + jars[i].getCanonicalPath() + "\" to \"" + newName.getCanonicalPath() + '"');
					}
					catch (Exception e)
					{
						try
						{
							logger.error("couldn't rename \"" + jars[i].getCanonicalPath() + "\" to \"" + newName.getCanonicalPath() + '"');
						}
						catch (IOException e1)
						{
						}
					}
				}
			}
		}
	}

}
