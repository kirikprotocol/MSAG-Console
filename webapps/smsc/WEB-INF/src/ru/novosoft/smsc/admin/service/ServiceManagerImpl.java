/*
 * Created by igork
 * Date: Feb 28, 2002
 * Time: 12:33:36 PM
 */
package ru.novosoft.smsc.admin.service;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.*;

import java.io.*;
import java.util.*;


public class ServiceManagerImpl implements ServiceManager
{
	//protected final static String SYSTEM_ID_PARAM_NAME = "system id";

	private Map services = new HashMap();
	private Category logger = Category.getInstance(this.getClass());

	public ServiceManagerImpl()
	{
	}

	public void addAll(Map newServices) throws AdminException
	{
		for (Iterator i = newServices.keySet().iterator(); i.hasNext();)
		{
			String serviceId = (String) i.next();
			if (contains(serviceId))
				throw new AdminException("Service \"" + serviceId + "\" already presented in system");
		}
		for (Iterator i = newServices.values().iterator(); i.hasNext();)
			add((Service) i.next());
	}

	private Service add(Service newService) throws AdminException
	{
		final String id = newService.getInfo().getId();
		if (services.containsKey(id))
			throw new AdminException("Service \"" + id + "\" already present");
		services.put(id, newService);
		return newService;
	}

	public boolean contains(String id)
	{
		return services.containsKey(id);
	}

	public List getServiceIds()
	{
		return new SortedList(services.keySet());
	}

	public void removeAll(Collection serviceIds) throws AdminException
	{
		requireAll(serviceIds);
		for (Iterator i = serviceIds.iterator(); i.hasNext();)
			remove((String) i.next());
	}

	public Service get(String serviceId) throws AdminException
	{
		require(serviceId);
		return (Service) services.get(serviceId);
	}

	public Service remove(String serviceId)
			throws AdminException
	{
		require(serviceId);
		Service s = get(serviceId);
		String host = s.getInfo().getHost();
		services.remove(serviceId);
		if (!Functions.recursiveDeleteFolder(WebAppFolders.getServiceFolder(host, serviceId))
				|| !Functions.recursiveDeleteFolder(WebAppFolders.getServiceJspsFolder(serviceId)))
			throw new AdminException("Service removed, but services files not deleted");
		return s;
	}


	/*************************************************************************************************************/

	public ServiceInfo getInfo(String servoceId)
			throws AdminException
	{
		Service s = get(servoceId);
		return s.getInfo();
	}

	/* *********************************** helpers ******************************/
	private void requireAll(Collection serviceIds) throws AdminException
	{
		for (Iterator i = serviceIds.iterator(); i.hasNext();)
			require((String) i.next());
	}

	private void require(String serviceId) throws AdminException
	{
		if (!contains(serviceId))
			throw new AdminException("Service \"" + serviceId + "\" not found");
	}

	public boolean isService(String smeId)
	{
		return services.keySet().contains(smeId);
	}

	public void deployAdministrableService(File incomingZip, ServiceInfo serviceInfo)
			throws AdminException
	{
		String hostName = serviceInfo.getHost();
		String serviceId = serviceInfo.getId();
		try
		{
			/****** deploy files ******/
			File serviceFolder = WebAppFolders.getServiceFolder(hostName, serviceId);
			File jspsFolder = WebAppFolders.getServiceJspsFolder(serviceId);

			if (serviceFolder.exists())
				throw new AdminException("Service already exists in filesystem");
			if (jspsFolder.exists())
				throw new AdminException("Jps pages for new services already exists");

			Functions.unZipArchive(serviceFolder,
										  new BufferedInputStream(new FileInputStream(incomingZip)));

			File incomingJsps = new File(serviceFolder, "jsp");
			if (!incomingJsps.renameTo(jspsFolder))
				throw new AdminException("Couldn't deploy JSP's (\"" + incomingJsps.getCanonicalPath() + "\") to \"" + jspsFolder.getCanonicalPath() + "\"");

			File newLogFolder = new File(serviceFolder, "log");
			newLogFolder.mkdir();
			moveJars(new File(serviceFolder, "lib"), WebAppFolders.getWebinfLibFolder());

			File deploy_config = new File(serviceFolder, "config.xml");
			if (deploy_config.exists() && deploy_config.isFile())
				deploy_config.delete();

			/****** register new sme *****/
			add(new Service(serviceInfo));
		}
		catch (AdminException e)
		{
			rollbackDeploy(hostName, serviceId);
			logger.error("Couldnt deploy new services", e);
			throw e;
		}
		catch (IOException e)
		{
			rollbackDeploy(hostName, serviceId);
			logger.error("Couldnt deploy new services", e);
			throw new AdminException("Couldnt deploy new services, nested: " + e.getMessage());
		}
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

	public void rollbackDeploy(String hostName, String serviceId)
	{
		services.remove(serviceId);

		File serviceFolder = WebAppFolders.getServiceFolder(hostName, serviceId);
		File jspsFolder = WebAppFolders.getServiceJspsFolder(serviceId);

		Functions.recursiveDeleteFolder(serviceFolder);
		Functions.recursiveDeleteFolder(jspsFolder);
	}

	public void updateServices(Map serviceInfos)
	{
		for (Iterator i = serviceInfos.values().iterator(); i.hasNext();)
		{
			ServiceInfo serviceInfo = (ServiceInfo) i.next();
			String id = serviceInfo.getId();
			Service service = (Service) services.get(id);
			if (service == null)
			{
				logger.warn("updateServices: Service \"" + id + "\" not found, create new one..");
				try
				{
					service = new Service(serviceInfo);
					services.put(id, service);
				}
				catch (AdminException e)
				{
					logger.error("updateServices: Couldn't create service \"" + id + "\"", e);
					continue;
				}
			}
			else
				service.setInfo(serviceInfo);
		}
		services.keySet().retainAll(serviceInfos.keySet());
	}

	public boolean isServiceAdministarble(String serviceId)
	{
		if (isService(serviceId))
		{
			final File jspsFolder = WebAppFolders.getServiceJspsFolder(serviceId);
			return jspsFolder.exists() && jspsFolder.isDirectory();
		}
		else
			return false;
	}
}
