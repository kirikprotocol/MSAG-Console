/*
 * Created by igork
 * Date: Feb 28, 2002
 * Time: 12:33:36 PM
 */
package ru.novosoft.smsc.admin.service;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
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
			  throws IsNotInitializedException
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


	private Map services = new HashMap();
	private DaemonManager daemonManager = null;
	protected static ConfigManager configManager = null;
	protected Category logger = Category.getInstance(this.getClass().getName());

	protected ServiceManager()
	{
		logger.debug("creating ServiceManager");
		daemonManager = new DaemonManager(smsc);
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
			ServiceInfo info = (ServiceInfo) newServices.get((String) i.next());
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
		if (services.keySet().contains(wizard.getSystemId()))
			throw new AdminException("Service \"" + wizard.getSystemId() + "\" already exists");
		else
		{
			wizard.deploy(daemonsFolder, webappFolder, webinfLibFolder);

			ServiceInfo serviceInfo = new ServiceInfo(wizard.getSystemId(), wizard.getHost(), wizard.getPort(),
																	wizard.getStartupArgs(), wizard.createSme());

			Daemon d = getDaemon(serviceInfo.getHost());
			if (services.containsKey(serviceInfo.getId()))
				throw new AdminException("Service \"" + serviceInfo.getId() + "\" already present");

			d.addService(serviceInfo);
			putService(new Service(serviceInfo));
			smsc.getSmes().add(serviceInfo.getSme());
			logger.debug("service added");
		}
	}

	public synchronized void addNonAdmService(AddNonAdmServiceWizard wizard)
			  throws AdminException
	{
		wizard.check();
		SME sme = wizard.createSme();
		if (smsc.getSmes().getNames().contains(sme.getId()))
			throw new AdminException("SME \"" + sme.getId() + "\" already exists");
		smsc.getSmes().add(sme);
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
		Service s = getService(serviceId);
		String host = s.getInfo().getHost();
		Daemon d = getDaemon(host);
		d.removeService(serviceId);
		services.remove(serviceId);
		smsc.getSmes().remove(serviceId);
		if (!recursiveDeleteFolder(getServiceFolder(host, serviceId))
				  || !recursiveDeleteFolder(getServiceJspsFolder(webappFolder, serviceId)))
			throw new AdminException("Service removed, but service files not deleted");
	}

	public synchronized void startService(String serviceId)
			  throws AdminException
	{
		Service s = getService(serviceId);
		Daemon d = getDaemon(s.getInfo().getHost());
		s.getInfo().setPid(d.startService(serviceId));
		if (s.getInfo().isRunning())
		{
			s.refreshComponents();
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
	 * Gets service IDs from specified host
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
		Set serviceIds = getServiceIds(hostName);
		int result = 0;
		for (Iterator i = serviceIds.iterator(); i.hasNext();)
		{
			String serviceId = (String) i.next();
			if (getServiceInfo(serviceId).isRunning())
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
		if (!s.getInfo().isRunning())
		{
			replaceService(new Service(new ServiceInfo(serviceId, d.getHost(), port, args, (SME) smsc.getSmes().get(serviceId))));
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
	}

	public synchronized void refreshService(String serviceId)
			  throws AdminException
	{
		Service s = getService(serviceId);
		Daemon d = daemonManager.getDaemon(s.getInfo().getHost());
		Map infos = d.listServices();
		for (Iterator j = infos.values().iterator(); j.hasNext();)
			replaceService(new Service((ServiceInfo) j.next()));
	}

	protected Service getService(String serviceId)
			  throws AdminException
	{
		Service s = (Service) services.get(serviceId);
		if (s == null)
			throw new AdminException("Unknown service \"" + serviceId + '"');
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
		if (s.getInfo().isRunning())
		{
			try
			{
				s.refreshComponents();
			}
			catch (AdminException e)
			{
				s.getInfo().setPid(0);
			}
		}
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

	static File getServiceJspsFolder(File webappFolder, String serviceId)
	{
		return new File(webappFolder, "esme_" + serviceId);
	}

	public synchronized List getSmeIds()
	{
		SMEList smes = smsc.getSmes();
		List smeIds = new Vector();
		for (Iterator i = smes.iterator(); i.hasNext(); )
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
}
