/*
 * Created by igork
 * Date: 26.09.2002
 * Time: 15:54:23
 */
package ru.novosoft.smsc.admin.service;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.SME;

public class AddNonAdmServiceWizard
{
	private String systemId = null;
	private String systemType = null;
	private int typeOfNumber = -1;
	private int numberingPlan = -1;
	private int interfaceVersion = 0x34;
	private String rangeOfAddress = null;
	private String password = null;

	public AddNonAdmServiceWizard(String systemId, String systemType, int typeOfNumber, int numberingPlan,
											int interfaceVersion, String password, String rangeOfAddress)
	{
		this.systemId = systemId;
		this.systemType = systemType;
		this.typeOfNumber = typeOfNumber;
		this.numberingPlan = numberingPlan;
		this.interfaceVersion = interfaceVersion;
		this.password = password;
		this.rangeOfAddress = rangeOfAddress;
	}

	public boolean check()
			  throws AdminException
	{
		return
				  systemId == null || systemId.length() == 0
				  || systemType == null || systemType.length() == 0
				  || typeOfNumber < 0
				  || numberingPlan < 0
				  || interfaceVersion != 0x34
				  || password == null
				  || rangeOfAddress == null || rangeOfAddress.length() == 0;
	}

	public SME createSme()
			  throws AdminException
	{
		return new SME(systemId, SME.SMPP, typeOfNumber, numberingPlan, interfaceVersion, systemType, password,
							rangeOfAddress, -1);
	}

	public String getSystemId()
	{
		return systemId;
	}

	public String getSystemType()
	{
		return systemType;
	}

	public int getTypeOfNumber()
	{
		return typeOfNumber;
	}

	public int getNumberingPlan()
	{
		return numberingPlan;
	}

	public int getInterfaceVersion()
	{
		return interfaceVersion;
	}

	public String getRangeOfAddress()
	{
		return rangeOfAddress;
	}
}
