package ru.novosoft.smsc.jsp.smsc.services;

import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.admin.route.SME;

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
	protected boolean disabled = false;
	protected byte mode = SME.MODE_TRX;

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

	protected void cleanup()
	{
		serviceId = null;
		priority = 0;
		systemType = "";
		typeOfNumber = 0;
		numberingPlan = 0;
		interfaceVersion = null;
		rangeOfAddress = "";
		password = "";
		timeout = 8;
		wantAlias = false;
		forceDC = false;
		receiptSchemeName = "default";
		disabled = false;
		mode = SME.MODE_TRX;
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

	public String getPriority()
	{
		return Integer.toString(priority);
	}

	public void setPriority(String priority)
	{
		try
		{
			this.priority = Integer.decode(priority).intValue();
		}
		catch (NumberFormatException e)
		{
			this.priority = 0;
		}
	}

	public String getSystemType()
	{
		return systemType;
	}

	public void setSystemType(String systemType)
	{
		this.systemType = systemType;
	}

	public int getTypeOfNumberInt()
	{
		return typeOfNumber;
	}

	public String getTypeOfNumber()
	{
		return Integer.toString(typeOfNumber);
	}

	public void setTypeOfNumber(String typeOfNumber)
	{
		try
		{
			this.typeOfNumber = Integer.decode(typeOfNumber).intValue();
		}
		catch (NumberFormatException e)
		{
			this.typeOfNumber = 0;
		}
	}

	public int getNumberingPlanInt()
	{
		return numberingPlan;
	}

	public String getNumberingPlan()
	{
		return Integer.toString(numberingPlan);
	}

	public void setNumberingPlan(String numberingPlan)
	{
		try
		{
			this.numberingPlan = Integer.decode(numberingPlan).intValue();
		}
		catch (NumberFormatException e)
		{
			this.numberingPlan = 0;
		}
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

	public String getTimeout()
	{
		return Integer.toString(timeout);
	}

	public void setTimeout(String timeout)
	{
		try
		{
			this.timeout = Integer.decode(timeout).intValue();
		}
		catch (NumberFormatException e)
		{
			this.timeout = 8;
		}
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

	public boolean isDisabled()
	{
		return disabled;
	}

	public void setDisabled(boolean disabled)
	{
		this.disabled = disabled;
	}

	public byte getModeByte()
	{
		return mode;
	}

	public String getMode()
	{
		return Byte.toString(mode);
	}

	public void setMode(String mode)
	{
		try
		{
			this.mode = Byte.decode(mode).byteValue();
		}
		catch (NumberFormatException e)
		{
			this.mode = 0;
		}
	}
}
