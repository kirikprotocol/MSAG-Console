/*
 * Created by igork
 * Date: 23.10.2002
 * Time: 22:50:27
 */
package ru.novosoft.smsc.jsp.smsc.hosts;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.daemon.Daemon;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.SmscBean;

import java.util.*;

public class HostView extends SmscBean
{
	public static final int RESULT_VIEW = PRIVATE_RESULT;
	public static final int RESULT_EDIT = PRIVATE_RESULT + 1;
	public static final int RESULT_ADD_SERVICE = PRIVATE_RESULT + 2;

	protected String hostName = null;
	protected String serviceIds[] = new String[0];
	protected String serviceId = null;

	protected Map services = null;
	protected Daemon daemon = null;

	protected String mbView = null;
	protected String mbEdit = null;
	protected String mbCancel = null;
	protected String mbAddService = null;
	protected String mbDelete = null;
	protected String mbStartService = null;
	protected String mbStopService = null;

	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

		daemon = daemonManager.getDaemon(hostName);
		if (daemon == null)
			return error(SMSCErrors.error.hosts.daemonNotFound, hostName);

		try
		{
			services = daemon.listServices();
			if (daemon.isContainsSmsc())
				services.remove(Constants.SMSC_SME_ID);
		}
		catch (AdminException e)
		{
			services = new HashMap();
			return error(SMSCErrors.warning.hosts.listFailed, hostName);
		}

		return RESULT_OK;
	}

	public int process(SMSCAppContext appContext, List errorMessages)
	{
		int result = super.process(appContext, errorMessages);
		if (result != RESULT_OK)
			return result;

		if (serviceIds == null)
			serviceIds = new String[0];

		if (mbAddService != null)
		{
			return RESULT_ADD_SERVICE;
		}
		else if (mbCancel != null)
		{
			return RESULT_DONE;
		}
		else if (mbEdit != null)
		{
			return RESULT_EDIT;
		}
		else if (mbDelete != null)
		{
			if (serviceIds.length == 0)
				return RESULT_OK;

			String sis = "";
			for (int i = 0; i < serviceIds.length; i++)
			{
				sis += serviceIds[i] + ", ";
			}
			return error("Delete services: " + sis);
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
		else
			return RESULT_OK;
	}

	protected int startServices()
	{
		int result = RESULT_OK;
		if (serviceIds.length == 0)
			return error(SMSCErrors.warning.hosts.noServicesSelected);

		List notStartedIds = new LinkedList();

		for (int i = 0; i < serviceIds.length; i++)
		{
			ServiceInfo s = (ServiceInfo) services.get(serviceIds[i]);
			if (s == null)
				result = error(SMSCErrors.error.hosts.serviceNotFound, serviceIds[i]);
			else if (s.getStatus() == ServiceInfo.STATUS_STOPPED)
			{
				long pid = -1;
				try
				{
					pid = daemon.startService(serviceIds[i]);
				}
				catch (AdminException e)
				{
					notStartedIds.add(serviceIds[i]);
					result = error(SMSCErrors.error.hosts.couldntStartService, serviceIds[i], e);
					logger.error("Couldn't start services \"" + serviceIds[i] + '"', e);
					continue;
				}
				if (pid <= 0)
				{
					notStartedIds.add(serviceIds[i]);
					result = error(SMSCErrors.error.hosts.couldntStartService, serviceIds[i]);
					logger.error("Couldn't start services \"" + serviceIds[i] + '"');
				}
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
			ServiceInfo s = (ServiceInfo) services.get(serviceIds[i]);
			if (s == null)
				result = error(SMSCErrors.error.hosts.serviceNotFound, serviceIds[i]);
			else if (s.getStatus() == ServiceInfo.STATUS_RUNNING)
			{
				try
				{
					daemon.shutdownService(serviceIds[i]);
				}
				catch (AdminException e)
				{
					notStoppedIds.add(serviceIds[i]);
					result = error(SMSCErrors.error.hosts.couldntStopService, serviceIds[i]);
					logger.error("Couldn't stop services \"" + serviceIds[i] + '"', e);
				}
			}
		}
		serviceIds = (String[]) notStoppedIds.toArray(new String[0]);
		return result;
	}

	public int getServicesTotal()
	{
		if (hostName != null && hostName.length() > 0)
		{
			try
			{
				return serviceManager.getCountServices(hostName) - (daemon.isContainsSmsc() ? 1 : 0);
			}
			catch (AdminException e)
			{
				logger.warn("host \"" + hostName + "\" not found", e);
			}
		}
		return 0;
	}

	public int getServicesRunning()
	{
		if (hostName != null && hostName.length() > 0)
		{
			try
			{
				return serviceManager.getCountRunningServices(hostName)
						- ((daemon.isContainsSmsc() && smsc.getInfo().getStatus() == ServiceInfo.STATUS_RUNNING) ? 1 : 0);
			}
			catch (AdminException e)
			{
				logger.warn("host \"" + hostName + "\" not found", e);
			}
		}
		return 0;
	}


	public Collection getServices()
	{
		return services.values();
	}

	public int getPort()
	{
		return daemon.getPort();
	}

	public String getHostName()
	{
		return hostName;
	}

	public void setHostName(String hostName)
	{
		this.hostName = hostName;
	}

	public String[] getServiceIds()
	{
		return serviceIds;
	}

	public void setServiceIds(String[] serviceIds)
	{
		this.serviceIds = serviceIds;
	}

	public String getMbView()
	{
		return mbView;
	}

	public void setMbView(String mbView)
	{
		this.mbView = mbView;
	}

	public String getMbEdit()
	{
		return mbEdit;
	}

	public void setMbEdit(String mbEdit)
	{
		this.mbEdit = mbEdit;
	}

	public String getMbCancel()
	{
		return mbCancel;
	}

	public void setMbCancel(String mbCancel)
	{
		this.mbCancel = mbCancel;
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

	public String getServiceId()
	{
		return serviceId;
	}

	public void setServiceId(String serviceId)
	{
		this.serviceId = serviceId;
	}
}
