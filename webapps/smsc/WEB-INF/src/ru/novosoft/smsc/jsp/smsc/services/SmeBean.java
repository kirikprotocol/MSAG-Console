package ru.novosoft.smsc.jsp.smsc.services;

import ru.novosoft.smsc.jsp.PageBean;

/**
 * Created by igork
 * Date: Feb 19, 2003
 * Time: 6:04:35 PM
 */
public class SmeBean extends PageBean
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
	protected String receiptSchemeName = "default";

	protected int convertInterfaceVersion(String version)
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

	public boolean isForceDC()
	{
		return forceDC;
	}

	public void setForceDC(boolean forceDC)
	{
		this.forceDC = forceDC;
	}

	public String getReceiptSchemeName()
	{
		return receiptSchemeName;
	}

	public void setReceiptSchemeName(String receiptSchemeName)
	{
		this.receiptSchemeName = receiptSchemeName;
	}
}
