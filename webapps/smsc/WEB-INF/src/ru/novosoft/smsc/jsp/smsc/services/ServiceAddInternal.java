/*
 * Created by igork
 * Date: 31.10.2002
 * Time: 6:21:34
 */
package ru.novosoft.smsc.jsp.smsc.services;

import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.admin.AdminException;

import java.util.List;

public class ServiceAddInternal extends PageBean
{
	protected String serviceId = "";
	protected String systemType = "";
	protected int typeOfNumber = 0;
	protected int numberingPlan = 0;
	protected String interfaceVersion = "3.4";
	protected String rangeOfAddress = "";
	protected String password = "";

	protected String hostName = null;

	protected String mbNext = null;
	protected String mbCancel = null;

	public int process(SMSCAppContext appContext, List errors)
	{
		int result = super.process(appContext, errors);
		if (result != RESULT_OK)
			return result;

		if (mbCancel != null)
			return RESULT_DONE;
		if (mbNext != null)
			return addService();

		return RESULT_OK;
	}

	protected int init(List errors)
	{
		return super.init(errors);
	}

	private int convertInterfaceVersion(String version)
	{
		int pos = version.indexOf('.');
		if (pos > 0)
		{
			return (Integer.parseInt(version.substring(0, pos)) << 4) + (Integer.parseInt(version.substring(pos+1)));
		}
		else
			return -1;
	}

	protected int addService()
	{
		try
		{
			serviceManager.addNonAdmService(serviceId, systemType, typeOfNumber, numberingPlan, convertInterfaceVersion(interfaceVersion), rangeOfAddress, password);
		}
		catch (Throwable t)
		{
			return error(SMSCErrors.error.services.coudntAddService, t);
		}
		return RESULT_DONE;
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

	public String getHostName()
	{
		return hostName;
	}

	public void setHostName(String hostName)
	{
		this.hostName = hostName;
	}
}
