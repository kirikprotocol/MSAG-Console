/*
 * Created by igork
 * Date: 23.10.2002
 * Time: 19:49:14
 */
package ru.novosoft.smsc.jsp.smsc.services;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.jsp.*;

import java.util.*;

public class Index extends PageBean
{
	public static final int RESULT_VIEW = PRIVATE_RESULT;
	public static final int RESULT_VIEW_HOST = PRIVATE_RESULT + 1;
	public static final int RESULT_ADD = PRIVATE_RESULT + 2;
	public static final int RESULT_EDIT = PRIVATE_RESULT + 3;

	protected String serviceId = null;
	protected String hostId = null;
	protected String serviceIds[] = new String[0];

	protected String mbAddService = null;
	protected String mbDelete = null;
	protected String mbStartService = null;
	protected String mbStopService = null;
	protected String mbView = null;
	protected String mbViewHost = null;
	protected String mbEdit = null;
	protected String mbDisconnectServices = null;


	public int process(SMSCAppContext appContext, List errors, java.security.Principal loginedPrincipal)
	{
		int result = super.process(appContext, errors, loginedPrincipal);
		if (result != RESULT_OK)
			return result;

		if (mbAddService != null)
			return RESULT_ADD;
		else if (mbDelete != null)
			return deleteServices();
		else if (mbStartService != null)
			return startServices();
		else if (mbStopService != null)
			return stopServices();
		else if (mbView != null)
			return RESULT_VIEW;
		else if (mbViewHost != null)
			return RESULT_VIEW_HOST;
		else if (mbEdit != null)
			return RESULT_EDIT;
		else if (mbDisconnectServices != null)
			return disconnectServices();
		else
			return RESULT_OK;
	}

	public Collection getSmeIds()
	{
		if (hostsManager == null)
		{
			error("Service Manager is null!!!");
			return new LinkedList();
		}
		else
		{
			List smeIds = hostsManager.getSmeIds();
			smeIds.remove(Constants.SMSC_SME_ID);
			return smeIds;
		}
	}

	public boolean isService(String smeId)
	{
		return hostsManager.isService(smeId);
	}

	public boolean isServiceAdministrable(String smeId)
	{
		return hostsManager.isServiceAdministarble(smeId);
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
				if (hostsManager.isService(id))
					hostsManager.removeService(id);
				else
					hostsManager.removeSme(id);
				//appContext.getStatuses().setServicesChanged(true);
			}
			catch (Throwable e)
			{
				error(SMSCErrors.error.services.coudntDeleteService, id);
				logger.error("Couldn't delete service \"" + id + '"', e);
				notRemoved.add(id);
			}
		}
		serviceIds = (String[]) notRemoved.toArray(new String[0]);
		return errors.size() == 0 ? RESULT_DONE : RESULT_ERROR;
	}

	protected int startServices()
	{
		logger.debug("startServices: " + (serviceIds != null ? serviceIds.length : 0));

		int result = RESULT_OK;
		if (serviceIds.length == 0)
			return error(SMSCErrors.warning.hosts.noServicesSelected);

		List notStartedIds = new LinkedList();

		for (int i = 0; i < serviceIds.length; i++)
		{
			final String serviceId = serviceIds[i];
			if (hostsManager.isService(serviceId))
			{
				try
				{
					if (hostsManager.getServiceInfo(serviceId).getStatus() == ServiceInfo.STATUS_STOPPED)
					{
						if (hostsManager.startService(serviceId) <= 0)
						{
							notStartedIds.add(serviceId);
							result = error(SMSCErrors.error.hosts.couldntStartService, serviceId);
							logger.error("Couldn't start services \"" + serviceId + '"');
						}
					}
					else
						logger.debug("startServices: " + serviceId + " is " + hostsManager.getServiceInfo(serviceId).getStatusStr());
				}
				catch (AdminException e)
				{
					notStartedIds.add(serviceId);
					result = error(SMSCErrors.error.hosts.couldntStartService, serviceId, e);
					logger.error("Couldn't start services \"" + serviceId + '"', e);
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
		logger.debug("stopServices: " + (serviceIds != null ? serviceIds.length : 0));

		int result = RESULT_OK;

		if (serviceIds.length == 0)
			return error(SMSCErrors.warning.hosts.noServicesSelected);

		List notStoppedIds = new LinkedList();

		for (int i = 0; i < serviceIds.length; i++)
		{
			final String serviceId = serviceIds[i];
			try
			{
				if (hostsManager.getServiceInfo(serviceId).getStatus() == ServiceInfo.STATUS_RUNNING)
					hostsManager.shutdownService(serviceId);
				else
					logger.debug("stopServices: " + serviceId + " is " + hostsManager.getServiceInfo(serviceId).getStatusStr());
			}
			catch (AdminException e)
			{
				notStoppedIds.add(serviceId);
				result = error(SMSCErrors.error.hosts.couldntStopService, serviceId);
				logger.error("Couldn't stop services \"" + serviceId + '"', e);
			}
		}
		serviceIds = (String[]) notStoppedIds.toArray(new String[0]);
		return result;
	}

	private int disconnectServices()
	{
		logger.debug("disconnectServices: " + (serviceIds != null ? serviceIds.length : 0));

		int result = RESULT_OK;

		if (serviceIds.length == 0)
			return error(SMSCErrors.warning.hosts.noServicesSelected);

		try
		{
			appContext.getSmeManager().disconnectSmes(Arrays.asList(serviceIds));
		}
		catch (AdminException e)
		{
			result = error(SMSCErrors.error.hosts.couldntStopService, serviceId);
			logger.error("Couldn't disconnect services \"" + serviceIds + '"', e);
		}
		serviceIds = new String[0];
		return result;
	}


	public String getHost(String sId)
	{
		try
		{
			return hostsManager.getServiceInfo(sId).getHost();
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
		if (hostsManager.isService(serviceId))
		{
			try
			{
				return hostsManager.getServiceInfo(serviceId).getStatus();
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

	public boolean isServiceDisabled(String serviceId)
	{
		try
		{
			return appContext.getSmeManager().get(serviceId).isDisabled();
		}
		catch (AdminException e)
		{
			error(SMSCErrors.error.services.couldntGetServiceInfo, serviceId);
			return false;
		}
	}

	public boolean isServiceConnected(String serviceId)
	{
		try
		{
			return appContext.getSmeManager().isSmeConnected(serviceId);
		}
		catch (AdminException e)
		{
			error(SMSCErrors.error.services.couldntGetServiceInfo, serviceId);
			return false;
		}
	}

	public boolean isSmscAlive()
	{
		try
		{
			return hostsManager.getServiceInfo(Constants.SMSC_SME_ID).getStatus() == ServiceInfo.STATUS_RUNNING;
		}
		catch (AdminException e)
		{
			error(SMSCErrors.error.services.couldntGetServiceInfo, Constants.SMSC_SME_ID);
			return false;
		}
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

	public String getMbEdit()
	{
		return mbEdit;
	}

	public void setMbEdit(String mbEdit)
	{
		this.mbEdit = mbEdit;
	}

	public String getMbDisconnectServices()
	{
		return mbDisconnectServices;
	}

	public void setMbDisconnectServices(String mbDisconnectServices)
	{
		this.mbDisconnectServices = mbDisconnectServices;
	}
}
