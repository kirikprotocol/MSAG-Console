/*
 * Created by igork
 * Date: Feb 28, 2002
 * Time: 12:33:36 PM
 */
package ru.novosoft.smsc.admin.service;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.daemon.Daemon;
import ru.novosoft.smsc.admin.daemon.DaemonManager;
import ru.novosoft.smsc.admin.route.SME;
import ru.novosoft.smsc.admin.route.SMEList;
import ru.novosoft.smsc.admin.smsc_service.Smsc;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.config.ConfigManager;

import java.io.*;
import java.util.*;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;


public class ServiceManager
{
	static public class IsNotInitializedException extends Exception
	{
		IsNotInitializedException(String s)
		{
			super(s);
		}
	}


	protected final static String SYSTEM_ID_PARAM_NAME = "system id";
	protected static ServiceManager serviceManager = null;
	private static boolean isInitialized = false;
	protected static File webappFolder = null;
	protected static File webinfFolder = null;
	protected static File webinfLibFolder = null;
	protected static File workFolder = null;
	protected static File daemonsFolder = null;
	protected static Smsc smsc = null;

	public static ServiceManager getInstance()
			throws IsNotInitializedException, AdminException
	{
		try
		{
			Category.getInstance(ServiceManager.class.getName()).debug("current folder: " + (new File(".")).getCanonicalPath());
		}
		catch (IOException e)
		{
		}
		if (!isInitialized)
			throw new IsNotInitializedException("Service Manager is not initialized. Make ServiceManager.Init(...) call before ServiceManager.getInstance()");
		if (serviceManager == null)
			serviceManager = new ServiceManager();
		return serviceManager;
	}

	public static void init(ConfigManager cfgManager, Smsc smscenter)
			throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		configManager = cfgManager;
		isInitialized = configManager != null;
		webappFolder = new File(configManager.getConfig().getString("system.webapp folder"));
		webinfFolder = new File(webappFolder, "WEB-INF");
		webinfLibFolder = new File(webinfFolder, "lib");
		workFolder = new File(webinfFolder, "work");
		daemonsFolder = new File(webinfFolder, "daemons");
		smsc = smscenter;
	}

	public static File getServiceFolder(String serviceId)
			throws AdminException
	{
		return new File(new File(daemonsFolder, serviceManager.getServiceInfo(serviceId).getHost()), serviceId);
	}


	private Map services = new HashMap();
	private DaemonManager daemonManager = null;
	protected static ConfigManager configManager = null;
	protected Category logger = Category.getInstance(this.getClass().getName());

	protected ServiceManager() throws AdminException
	{
		logger.debug("creating ServiceManager");
		daemonManager = new DaemonManager(smsc);
		putService(smsc);
		Config config = configManager.getConfig();
		Set daemons = config.getSectionChildSectionNames("daemons");
		for (Iterator i = daemons.iterator(); i.hasNext();)
		{
			String encodedName = (String) i.next();
			String daemonName = StringEncoderDecoder.decodeDot(encodedName.substring(encodedName.lastIndexOf('.') + 1));
			try
			{
				addDaemonInternal(daemonName, config.getInt(encodedName + ".port"));
			}
			catch (AdminException e)
			{
				logger.error("Couldn't add daemon \"" + encodedName + "\"", e);
			}
			catch (Config.ParamNotFoundException e)
			{
				logger.debug("Misconfigured daemon \"" + encodedName + "\", parameter port missing", e);
			}
			catch (Config.WrongParamTypeException e)
			{
				logger.debug("Misconfigured daemon \"" + encodedName + "\", parameter port misformatted", e);
			}
		}
	}

	protected void addDaemonInternal(String host, int port)
			throws AdminException
	{
		Daemon d = daemonManager.addDaemon(host, port);
		Map newServices = d.listServices();
		for (Iterator i = newServices.keySet().iterator(); i.hasNext();)
		{
			ServiceInfo info = (ServiceInfo) newServices.get(i.next());
			if (info.getId().equals(Constants.SMSC_SME_ID))
				smsc.setInfo(info);
			else
				putService(new Service(info));
		}
	}

	public synchronized void addDaemon(String host, int port)
			throws AdminException
	{
		addDaemonInternal(host, port);
		Config config = configManager.getConfig();
		config.setInt("daemons." + StringEncoderDecoder.encodeDot(host) + ".port", port);
		try
		{
			configManager.save();
		}
		catch (Exception e)
		{
			logger.error("Couldn't save config", e);
		}
	}

	public synchronized Set getHostNames()
	{
		return daemonManager.getHosts();
	}

	protected File saveFileToTemp(InputStream in)
			throws IOException
	{
		File tmpFile = File.createTempFile("SMSC_SME_distrib_", ".zip.tmp");
		OutputStream out = new BufferedOutputStream(new FileOutputStream(tmpFile));

		byte buffer[] = new byte[2048];
		for (int readed = 0; (readed = in.read(buffer)) > -1;)
		{
			out.write(buffer, 0, readed);
		}

		in.close();
		out.close();
		return tmpFile;
	}

	public synchronized AddAdmServiceWizard receiveNewServiceArchive(InputStream in)
			throws AdminException
	{
		AddAdmServiceWizard wizard = new AddAdmServiceWizard(in);
		if (services.keySet().contains(wizard.getSystemId()))
			throw new AdminException("Service \"" + wizard.getSystemId() + "\" already exists");
		else
			return wizard;
	}

	protected File getServiceFolder(String host, String serviceId)
	{
		return new File(new File(daemonsFolder, host), serviceId);
	}

	public synchronized void addAdmService(AddAdmServiceWizard wizard)
			throws AdminException
	{
		if (services.containsKey(wizard.getSystemId()))
			throw new AdminException("Service \"" + wizard.getSystemId() + "\" already exists");
		else if (smsc.getSmes().contains(wizard.getSystemId()))
			throw new AdminException("SME \"" + wizard.getSystemId() + "\" already exists");
		else
		{
			wizard.deploy(daemonsFolder, webappFolder, webinfLibFolder);

			ServiceInfo serviceInfo = new ServiceInfo(wizard.getSystemId(), wizard.getHost(), wizard.getPort(),
																	wizard.getStartupArgs(), wizard.createSme(), ServiceInfo.STATUS_STOPPED);

			Daemon d = getDaemon(serviceInfo.getHost());
			d.addService(serviceInfo);
			putService(new Service(serviceInfo));
			smsc.getSmes().add(serviceInfo.getSme());
			//smsc.saveSmesConfig();
			logger.debug("services added");
		}
	}

	public synchronized void addNonAdmService(String serviceId,
															int priority,
															String systemType,
															int typeOfNumber,
															int numberingPlan,
															int interfaceVersion,
															String rangeOfAddress,
															String password,
															boolean wantAlias,
															int timeout
															)
			throws AdminException
	{
		SME sme = new SME(serviceId, priority, SME.SMPP, typeOfNumber, numberingPlan, interfaceVersion, systemType, password,
								rangeOfAddress, -1, wantAlias, timeout);
		if (smsc.getSmes().getNames().contains(sme.getId()))
			throw new AdminException("SME \"" + sme.getId() + "\" already exists");
		smsc.getSmes().add(sme);
		//smsc.saveSmesConfig();
	}

	protected boolean recursiveDeleteFolder(File folder)
	{
		String[] childNames = folder.list();
		if (childNames != null)
		{
			for (int i = 0; i < childNames.length; i++)
			{
				File child = new File(folder, childNames[i]);
				if (child.isDirectory())
					recursiveDeleteFolder(child);
				else
					child.delete();
			}
		}
		return folder.delete();
	}

	public synchronized void removeService(String serviceId)
			throws AdminException
	{
		if (smsc.isSmeUsed(serviceId))
			throw new AdminException("Couldn't remove service \"" + serviceId + "\" becouse it is used by routes");

		Service s = getService(serviceId);
		String host = s.getInfo().getHost();
		Daemon d = getDaemon(host);
		d.removeService(serviceId);
		services.remove(serviceId);
		smsc.getSmes().remove(serviceId);
		smsc.saveSmesConfig();
		if (!recursiveDeleteFolder(getServiceFolder(host, serviceId))
				|| !recursiveDeleteFolder(getServiceJspsFolder(webappFolder, serviceId)))
			throw new AdminException("Service removed, but services files not deleted");
	}

	public synchronized void removeSme(String smeId)
			throws AdminException
	{
		if (isService(smeId))
			throw new AdminException("Couldn't remove sme \"" + smeId + "\" becouse it is service");

		if (smsc.isSmeUsed(smeId))
			throw new AdminException("Couldn't remove sme \"" + smeId + "\" becouse it is used by routes");

		smsc.getSmes().remove(smeId);
	}

	public synchronized void startService(String serviceId)
			throws AdminException
	{
		try
		{
			Service s = getService(serviceId);
			Daemon d = getDaemon(s.getInfo().getHost());
			s.getInfo().setPid(d.startService(serviceId));
			refreshService(serviceId);
		}
		catch (AdminException e)
		{
			//refreshService(serviceId);
			throw e;
		}
	}

	public synchronized void killService(String serviceId)
			throws AdminException
	{
		Service s = getService(serviceId);
		Daemon d = getDaemon(s.getInfo().getHost());

		d.killService(serviceId);
		refreshService(serviceId);
	}

	public synchronized void shutdownService(String serviceId)
			throws AdminException
	{
		getDaemon(getService(serviceId).getInfo().getHost()).shutdownService(serviceId);
	}

	public Object callServiceMethod(String hostName, String serviceId, String componentName,
											  String methodName, String returnTypeName, Map args)
			throws AdminException
	{
		Service s = getService(serviceId);
		if (!s.getInfo().getHost().equals(hostName))
			throw new AdminException("Wrong host name (\"" + hostName + "\")");
		Component c = (Component) s.getInfo().getComponents().get(componentName);
		if (c == null)
			throw new AdminException("Wrong component name (\"" + componentName + "\")");
		Method m = (Method) c.getMethods().get(methodName);
		if (m == null)
			throw new AdminException("Wrong method name (\"" + methodName + "\")");
		Type t = Type.getInstance(returnTypeName);
		if (!m.getType().equals(t))
			throw new AdminException("Wrong method return type (\"" + returnTypeName + "\")");
		Map params = m.getParams();
		if (!params.keySet().equals(args.keySet()))
			throw new AdminException("Wrong arguments");
		Map arguments = new HashMap();
		for (Iterator i = params.values().iterator(); i.hasNext();)
		{
			Parameter p = (Parameter) i.next();
			if (args.get(p.getName()) == null)
				throw new AdminException("Parameter \"" + p.getName() + "\" not specified");
			switch (p.getType().getId())
			{
				case Type.StringType:
					{
						arguments.put(p.getName(), args.get(p.getName()));
						break;
					}
				case Type.IntType:
					{
						arguments.put(p.getName(), Integer.decode((String) args.get(p.getName())));
						break;
					}
				case Type.BooleanType:
					{
						arguments.put(p.getName(), Boolean.valueOf((String) args.get(p.getName())));
						break;
					}
				default:
					{
						throw new AdminException("Unknown parameter \"" + p.getName() + "\" type \"" + p.getType().getName() + "\"");
					}
			}
		}
		return s.call(c, m, t, arguments);
	}

/*
	public synchronized Set getServiceIds()
	{
		return services.keySet();
	}
*/

	/**
	 * Gets services IDs from specified host
	 * @param host Host to lookup for services
	 * @return Service IDs (<code>String</code>s), that registered on specified host
	 */
	public synchronized Set getServiceIds(String host)
			throws AdminException
	{
		if (!getHostNames().contains(host))
			throw new AdminException("Host \"" + host + "\" not connected");

		Set result = new HashSet();
		for (Iterator i = services.keySet().iterator(); i.hasNext();)
		{
			Service s = getService((String) i.next());
			if (s.getInfo().getHost().equals(host))
				result.add(s.getInfo().getId());
		}
		return result;
	}

	public synchronized ServiceInfo getServiceInfo(String servoceId)
			throws AdminException
	{
		Service s = getService(servoceId);
		return s.getInfo();
	}

	public synchronized void removeDaemon(String host)
			throws AdminException
	{
		daemonManager.removeDaemon(host);
		refreshServices();
		Config config = configManager.getConfig();
		config.removeParam("daemons." + StringEncoderDecoder.encode(host) + ".port");
		try
		{
			configManager.save();
		}
		catch (Exception e)
		{
			logger.error("Couldn't save config", e);
		}
	}

	public synchronized int getCountRunningServices(String hostName)
			throws AdminException
	{
		refreshServices();
		Set serviceIds = getServiceIds(hostName);
		int result = 0;
		for (Iterator i = serviceIds.iterator(); i.hasNext();)
		{
			String serviceId = (String) i.next();
			if (getServiceInfo(serviceId).getStatus() == ServiceInfo.STATUS_RUNNING)
				result++;
		}
		return result;
	}

	public synchronized int getCountServices(String hostName)
			throws AdminException
	{
		return getServiceIds(hostName).size();
	}

	public synchronized void setStartupParameters(String serviceId, String host,
																 /*String serviceName, */int port, String args)
			throws AdminException
	{
		Service s = getService(serviceId);
		Daemon d = getDaemon(s.getInfo().getHost());
		d.setServiceStartupParameters(serviceId, /*serviceName, */port, args);
		if (s.getInfo().getStatus() == ServiceInfo.STATUS_STOPPED)
		{
			replaceService(new Service(new ServiceInfo(serviceId, d.getHost(), port, args, smsc.getSmes().get(serviceId), s.getInfo().getStatus())));
		}
	}

	/************************************ helpers ******************************/
	public synchronized void refreshServices()
			throws AdminException
	{
		services.clear();
		for (Iterator i = daemonManager.getHosts().iterator(); i.hasNext();)
		{
			Daemon d = daemonManager.getDaemon((String) i.next());
			Map infos = d.listServices();
			for (Iterator j = infos.values().iterator(); j.hasNext();)
				putService(new Service((ServiceInfo) j.next()));
		}
		putService(smsc);
	}

	public synchronized void refreshService(String serviceId)
			throws AdminException
	{
		Service s = getService(serviceId);
		Daemon d = daemonManager.getDaemon(s.getInfo().getHost());
		if (d == null)
			throw new AdminException("Daemon \"" + s.getInfo().getHost() + "\" not found");
		Map infos = d.listServices();
		for (Iterator j = infos.values().iterator(); j.hasNext();)
		{
			final ServiceInfo info = (ServiceInfo) j.next();
			logger.debug("Refresh \"" + serviceId + "\" service: \"" + info.getId() + '"');
			if (!info.getId().equals(Constants.SMSC_SME_ID))
				replaceService(new Service(info));
			if (info.getId().equals(serviceId))
				s.setInfo(info);
		}
	}

	protected Service getService(String serviceId)
			throws AdminException
	{
		Service s = (Service) services.get(serviceId);
		if (s == null)
			throw new AdminException("Unknown services \"" + serviceId + '"');
		return s;
	}

	protected void putService(Service s)
			throws AdminException
	{
		if (services.containsKey(s.getInfo().getId()))
			throw new AdminException("Service \"" + s.getInfo().getId() + "\" already present");
		replaceService(s);
	}

	protected void replaceService(Service s)
			throws AdminException
	{
		services.put(s.getInfo().getId(), s);
		//refreshService(s.getInfo().getId());
	}

	protected Daemon getDaemon(String hostName)
			throws AdminException
	{
		Daemon d = daemonManager.getDaemon(hostName);
		if (d == null)
			throw new AdminException("Host \"" + hostName + "\" not connected");
		return d;
	}

	protected void putDaemon(Daemon d)
			throws AdminException
	{
		if (services.containsKey(d.getHost()))
			throw new AdminException("Host \"" + d.getHost() + "\" already connected");
		services.put(d.getHost(), d);
	}

	public static File getServiceJspsFolder(File webappFolder, String serviceId)
	{
		return new File(webappFolder, "esme_" + serviceId);
	}

	public synchronized List getSmeIds()
	{
		SMEList smes = smsc.getSmes();
		List smeIds = new Vector();
		for (Iterator i = smes.iterator(); i.hasNext();)
		{
			SME sme = (SME) i.next();
			smeIds.add(sme.getId());
		}
		return smeIds;
	}

	public synchronized boolean isService(String smeId)
	{
		return services.keySet().contains(smeId);
	}

	public DaemonManager getDaemonManager()
	{
		return daemonManager;
	}

	public void deployAdministrableService(File incomingZip, ServiceInfo serviceInfo)
			throws AdminException
	{
		String hostName = serviceInfo.getHost();
		String serviceId = serviceInfo.getId();
		try
		{
			/****** deploy files ******/
			File daemonFolder = new File(daemonsFolder, hostName);
			File serviceFolder = new File(daemonFolder, serviceId);
			File jspsFolder = getServiceJspsFolder(webappFolder, serviceId);

			if (serviceFolder.exists())
				throw new AdminException("Service already exists in filesystem");
			if (jspsFolder.exists())
				throw new AdminException("Jps pages for new services already exists");

			unZipArchive(serviceFolder,
							 new BufferedInputStream(new FileInputStream(incomingZip)));

			File incomingJsps = new File(serviceFolder, "jsp");
			if (!incomingJsps.renameTo(jspsFolder))
				throw new AdminException("Couldn't deploy JSP's (\"" + incomingJsps.getCanonicalPath() + "\") to \"" + jspsFolder.getCanonicalPath() + "\"");

			File newLogFolder = new File(serviceFolder, "log");
			newLogFolder.mkdir();
			moveJars(new File(serviceFolder, "lib"), webinfLibFolder);

			File deploy_config = new File(serviceFolder, "config.xml");
			if (deploy_config.exists() && deploy_config.isFile())
				deploy_config.delete();

			/****** register new sme *****/
			putService(new Service(serviceInfo));
			smsc.getSmes().add(serviceInfo.getSme());
			smsc.saveSmesConfig();
		}
		catch (AdminException e)
		{
			rollbackDeploy(hostName, serviceId, daemonsFolder, webappFolder, webinfLibFolder);
			logger.error("Couldnt deploy new services", e);
			throw e;
		}
		catch (IOException e)
		{
			rollbackDeploy(hostName, serviceId, daemonsFolder, webappFolder, webinfLibFolder);
			logger.error("Couldnt deploy new services", e);
			throw new AdminException("Couldnt deploy new services, nested: " + e.getMessage());
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
			if (!e.isDirectory())
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

	protected void rollbackDeploy(String hostName, String serviceId, File daemonsFolder, File webappFolder, File webinfLibFolder)
	{
		File daemonFolder = new File(daemonsFolder, hostName);
		File serviceFolder = new File(daemonFolder, serviceId);
		File jspsFolder = ServiceManager.getServiceJspsFolder(webappFolder, serviceId);

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

	public boolean isServiceAdministarble(String serviceId)
	{
		if (isService(serviceId))
		{
			final File jspsFolder = getServiceJspsFolder(webappFolder, serviceId);
			return jspsFolder.exists() && jspsFolder.isDirectory();
		}
		else
			return false;
	}
}
