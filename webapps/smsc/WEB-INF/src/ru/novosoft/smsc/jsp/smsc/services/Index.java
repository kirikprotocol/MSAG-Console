/*
 * Created by igork
 * Date: 23.10.2002
 * Time: 19:49:14
 */
package ru.novosoft.smsc.jsp.smsc.services;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.daemon.Daemon;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;

import java.util.Collection;
import java.util.LinkedList;
import java.util.List;

public class Index extends PageBean
{
	public static final int RESULT_VIEW = PRIVATE_RESULT;
	public static final int RESULT_VIEW_HOST = PRIVATE_RESULT + 1;
	public static final int RESULT_ADD = PRIVATE_RESULT + 2;

	protected String serviceId = null;
	protected String hostId = null;
	protected String serviceIds[] = new String[0];

	protected String mbAddService = null;
	protected String mbDelete = null;
	protected String mbStartService = null;
	protected String mbStopService = null;
	protected String mbView = null;
	protected String mbViewHost = null;


	public int process(SMSCAppContext appContext, List errors)
	{
		int result = super.process(appContext, errors);
		if (result != RESULT_OK)
			return result;

		if (mbAddService != null)
		{
			return RESULT_ADD;
		}
		else if (mbDelete != null)
		{
			return deleteServices();
		}
		else if (mbStartService != null)
		{
			return startServices();
		}
		else if (mbStopService != null)
		{
			return stopServices();
		}
		else if (mbView != null)
		{
			return RESULT_VIEW;
		}
		else if (mbViewHost != null)
		{
			return RESULT_VIEW_HOST;
		}
		else
			return RESULT_OK;
	}

	public Collection getSmeIds()
	{
		if (serviceManager == null)
		{
			error("Service Manager is null!!!");
			return new LinkedList();
		}
		else
			return serviceManager.getSmeIds();
	}

	public boolean isService(String smeId)
	{
		return serviceManager.isService(smeId);
	}

	public boolean isServiceAdministrable(String smeId)
	{
		return serviceManager.isServiceAdministarble(smeId);
	}

	/************************ Command handlers ****************************/

	protected int deleteServices()
	{
		List notRemoved = new LinkedList();
		for (int i = 0; i < serviceIds.length; i++)
		{
			String id = serviceIds[i];
			try
			{
				if (serviceManager.isService(id))
					serviceManager.removeService(id);
				else
					serviceManager.removeSme(id);
				appContext.getStatuses().setServicesChanged(true);
			}
			catch (Throwable e)
			{
				error(SMSCErrors.error.services.coudntDeleteService, id);
				logger.error("Couldn't delete service \"" + id + '"', e);
				notRemoved.add(id);
			}
		}
		serviceIds = (String[]) notRemoved.toArray(new String[0]);
		return errors.size() == 0 ? RESULT_OK : RESULT_ERROR;
	}

	protected int startServices()
	{
		int result = RESULT_OK;
		if (serviceIds.length == 0)
			return error(SMSCErrors.warning.hosts.noServicesSelected);

		List notStartedIds = new LinkedList();

		for (int i = 0; i < serviceIds.length; i++)
		{
			if (serviceManager.isService(serviceIds[i]))
			{
				try
				{
					ServiceInfo s = serviceManager.getServiceInfo(serviceIds[i]);
					if (s == null)
						result = error(SMSCErrors.error.hosts.serviceNotFound, serviceIds[i]);
					else if (s.getStatus() == ServiceInfo.STATUS_STOPPED)
					{
						long pid = -1;
						Daemon daemon = daemonManager.getDaemon(s.getHost());
						pid = daemon.startService(serviceIds[i]);
						if (pid <= 0)
						{
							notStartedIds.add(serviceIds[i]);
							result = error(SMSCErrors.error.hosts.couldntStartService, serviceIds[i]);
							logger.error("Couldn't start services \"" + serviceIds[i] + '"');
						}
					}
				}
				catch (AdminException e)
				{
					notStartedIds.add(serviceIds[i]);
					result = error(SMSCErrors.error.hosts.couldntStartService, serviceIds[i], e);
					logger.error("Couldn't start services \"" + serviceIds[i] + '"', e);
					continue;
				}
			}
			else
			{
				notStartedIds.add(serviceId);
				result = error(SMSCErrors.error.services.couldntStartInternalService, serviceId);
			}
		}
		serviceIds = (String[]) notStartedIds.toArray(new String[0]);
		return result;
	}

	protected int stopServices()
	{
		int result = RESULT_OK;

		if (serviceIds.length == 0)
			return error(SMSCErrors.warning.hosts.noServicesSelected);

		List notStoppedIds = new LinkedList();

		for (int i = 0; i < serviceIds.length; i++)
		{
			try
			{
				ServiceInfo s = serviceManager.getServiceInfo(serviceIds[i]);
				if (s == null)
					result = error(SMSCErrors.error.hosts.serviceNotFound, serviceIds[i]);
				else if (s.getStatus() == ServiceInfo.STATUS_RUNNING)
				{
					Daemon daemon = daemonManager.getDaemon(s.getHost());
					daemon.shutdownService(serviceIds[i]);
				}
			}
			catch (AdminException e)
			{
				notStoppedIds.add(serviceIds[i]);
				result = error(SMSCErrors.error.hosts.couldntStopService, serviceIds[i]);
				logger.error("Couldn't stop services \"" + serviceIds[i] + '"', e);
			}
		}
		serviceIds = (String[]) notStoppedIds.toArray(new String[0]);
		return result;
	}

	public String getHost(String sId)
	{
		try
		{
			return serviceManager.getServiceInfo(sId).getHost();
		}
		catch (Throwable e)
		{
			error(SMSCErrors.error.services.couldntGetServiceInfo, e);
			logger.error("Couldn't get service info for service \"" + sId + '"', e);
			return "";
		}
	}

	public byte getServiceStatus(String serviceId)
	{
		if (serviceManager.isService(serviceId))
		{
			try
			{
				return serviceManager.getServiceInfo(serviceId).getStatus();
			}
			catch (Throwable t)
			{
				logger.error("Couldn't get service info for service \"" + serviceId + '"', t);
				error(SMSCErrors.error.services.couldntGetServiceInfo, serviceId);
				return ServiceInfo.STATUS_UNKNOWN;
			}
		}
		else
			return ServiceInfo.STATUS_RUNNING;
	}

	/*************************** Properties *******************************/


	public String getServiceId()
	{
		return serviceId;
	}

	public void setServiceId(String serviceId)
	{
		this.serviceId = serviceId;
	}

	public String getHostId()
	{
		return hostId;
	}

	public void setHostId(String hostId)
	{
		this.hostId = hostId;
	}

	public String[] getServiceIds()
	{
		return serviceIds;
	}

	public void setServiceIds(String[] serviceIds)
	{
		this.serviceIds = serviceIds;
	}

	public String getMbAddService()
	{
		return mbAddService;
	}

	public void setMbAddService(String mbAddService)
	{
		this.mbAddService = mbAddService;
	}

	public String getMbDelete()
	{
		return mbDelete;
	}

	public void setMbDelete(String mbDelete)
	{
		this.mbDelete = mbDelete;
	}

	public String getMbStartService()
	{
		return mbStartService;
	}

	public void setMbStartService(String mbStartService)
	{
		this.mbStartService = mbStartService;
	}

	public String getMbStopService()
	{
		return mbStopService;
	}

	public void setMbStopService(String mbStopService)
	{
		this.mbStopService = mbStopService;
	}

	public String getMbView()
	{
		return mbView;
	}

	public void setMbView(String mbView)
	{
		this.mbView = mbView;
	}

	public String getMbViewHost()
	{
		return mbViewHost;
	}

	public void setMbViewHost(String mbViewHost)
	{
		this.mbViewHost = mbViewHost;
	}
}
