package ru.novosoft.smsc.jsp.smsc.services;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.SME;
import ru.novosoft.smsc.jsp.*;

import java.util.List;

/**
 * Created by igork
 * Date: Jan 16, 2003
 * Time: 7:00:32 PM
 */
public class ServiceEditSme extends PageBean
{
	protected String serviceId = null;
	protected int priority = 0;
	protected String systemType = "";
	protected int typeOfNumber = 0;
	protected int numberingPlan = 0;
	protected String interfaceVersion = null;
	protected String rangeOfAddress = "";
	protected String password = "";
	protected int timeout = 8;
	protected boolean wantAlias = false;
	protected boolean forceDC = false;

	protected String mbSave = null;
	protected String mbCancel = null;

	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

		if (serviceId == null || serviceId.length() == 0)
			return error(SMSCErrors.error.services.ServiceIdNotDefined);

		if (interfaceVersion == null)
		{
			SME sme = null;
			try
			{
				sme = appContext.getSmeManager().getSmes().get(serviceId);
			}
			catch (AdminException e)
			{
				logger.error("Couldn't get sme \"" + serviceId + "\"");
				return error(SMSCErrors.error.services.ServiceNotFound, serviceId);
			}
			if (sme == null)
				return error(SMSCErrors.error.services.ServiceNotFound, serviceId);

			priority = sme.getPriority();
			systemType = sme.getSystemType();
			typeOfNumber = sme.getTypeOfNumber();
			numberingPlan = sme.getNumberingPlan();
			interfaceVersion = String.valueOf(sme.getInterfaceVersion() >> 4) + '.' + String.valueOf(sme.getInterfaceVersion() & 0xF);
			rangeOfAddress = sme.getAddrRange();
			password = sme.getPassword();
			timeout = sme.getTimeout();
			wantAlias = sme.isWantAlias();
			forceDC = sme.isForceDC();
		}

		return RESULT_OK;
	}

	public int process(SMSCAppContext appContext, List errors)
	{
		int result = super.process(appContext, errors);
		if (result != RESULT_OK)
			return result;

		if (serviceId == null)
		{
			serviceId = "";
			wantAlias = true;
		}

		if (mbCancel != null)
			return RESULT_DONE;
		if (mbSave != null)
			return save();

		return RESULT_OK;
	}

	private int save()
	{
		if (serviceId == null || serviceId.length() == 0)
			return error(SMSCErrors.error.services.ServiceIdNotDefined);
		if (!hostsManager.getSmeIds().contains(serviceId))
			return error(SMSCErrors.error.services.ServiceIdNotDefined, serviceId);
		if (serviceId.length() > 15)
			return error(SMSCErrors.error.services.ServiceIdTooLong);
		if (priority < 0 || priority > MAX_PRIORITY)
			return error(SMSCErrors.error.services.invalidPriority, String.valueOf(priority));

		try
		{
			SME sme = new SME(serviceId, priority, SME.SMPP, typeOfNumber, numberingPlan, convertInterfaceVersion(interfaceVersion), systemType, password, rangeOfAddress, -1, wantAlias, forceDC, timeout);
			if (hostsManager.isService(serviceId))
				hostsManager.getServiceInfo(serviceId).setSme(sme);
			appContext.getSmeManager().getSmes().remove(serviceId);
			appContext.getSmeManager().getSmes().add(sme);
			appContext.getStatuses().setServicesChanged(true);
		}
		catch (Throwable t)
		{
			logger.error("Couldn't update SME parameters", t);
			return error(SMSCErrors.error.services.coudntAddService, t);
		}
		return RESULT_DONE;
	}

	private int convertInterfaceVersion(String version)
	{
		int pos = version.indexOf('.');
		if (pos > 0)
		{
			return (Integer.parseInt(version.substring(0, pos)) << 4) + (Integer.parseInt(version.substring(pos + 1)));
		}
		else
			return -1;
	}

	/******************************* properties ****************************************/
	public String getServiceId()
	{
		return serviceId;
	}

	public void setServiceId(String serviceId)
	{
		this.serviceId = serviceId;
	}

	public int getPriority()
	{
		return priority;
	}

	public void setPriority(int priority)
	{
		this.priority = priority;
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
		return interfaceVersion;
	}

	public void setInterfaceVersion(String interfaceVersion)
	{
		this.interfaceVersion = interfaceVersion;
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

	public int getTimeout()
	{
		return timeout;
	}

	public void setTimeout(int timeout)
	{
		this.timeout = timeout;
	}

	public boolean isWantAlias()
	{
		return wantAlias;
	}

	public void setWantAlias(boolean wantAlias)
	{
		this.wantAlias = wantAlias;
	}

	public String getMbSave()
	{
		return mbSave;
	}

	public void setMbSave(String mbSave)
	{
		this.mbSave = mbSave;
	}

	public String getMbCancel()
	{
		return mbCancel;
	}

	public void setMbCancel(String mbCancel)
	{
		this.mbCancel = mbCancel;
	}

	public boolean isForceDC()
	{
		return forceDC;
	}

	public void setForceDC(boolean forceDC)
	{
		this.forceDC = forceDC;
	}
}
