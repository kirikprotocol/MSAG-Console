/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 13:50:54
 */
package ru.novosoft.smsc.admin.route;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;


public class SME
{
	public static final byte SMPP = 0;
	public static final byte SS7 = 1;

	private String id = null;
	private int priority = 0;
	private byte type = SMPP;
	private int typeOfNumber = 0;
	private int numberingPlan = 0;
	private int interfaceVersion = 0;
	private String systemType = null;
	private String password = null;
	private String addrRange = null;
	private int smeN = 0;
	private boolean wantAlias = false;
	private int timeout = 0;
	private boolean forceDC = false;
	private String receiptSchemeName = null;

	/**
	 * Constructor
	 * @param id уникальный идентификатор SME
	 * @param priority приоритет этого SME
	 * @param type тип SME. может быть SME.SMPP или SME.SS7
	 * @param typeOfNumber тип номера SME? возможные значени€: <dl compact>
	 * <dt>0</dt><dd>Unknown</dd>
	 * <dt>1</dt><dd>International</dd>
	 * <dt>2</dt><dd>National</dd>
	 * <dt>3</dt><dd>Network Specific</dd>
	 * <dt>4</dt><dd>Subscriber Number</dd>
	 * <dt>5</dt><dd>Alphanumeric</dd>
	 * <dt>6</dt><dd>Abbreviated</dd>
	 * </dl>
	 * @param numberingPlan возможные значени€: <dl compact>
	 * <dt>0</dt><dd>Unknown</dd>
	 * <dt>1</dt><dd>ISDN (E163/E164)</dd>
	 * <dt>3</dt><dd>Data (X.121)</dd>
	 * <dt>4</dt><dd>Telex (F.69)</dd>
	 * <dt>6</dt><dd>Land Mobile (E.212)</dd>
	 * <dt>8</dt><dd>National</dd>
	 * <dt>9</dt><dd>Private</dd>
	 * <dt>10</dt><dd>ERMES</dd>
	 * <dt>14</dt><dd>Internet (IP)</dd>
	 * <dt>18</dt><dd>WAP Client Id (to be defined by WAP Forum)</dd>
	 * </dl>
	 * @param interfaceVersion ¬озможные значени€: 0x34 (v3.4)
	 * @param systemType <font color=red>???</font>
	 * @param password пароль дл€ доступа SME к SMSC
	 * @param addrRange <font color=red>???</font>
	 * @param smeN <font color=red>???</font>
	 * @param wantAlias
	 * @param forceDC
	 * @param timeout
	 * @param receiptSchemeName ASCIIZ[32] с именем схемы отчетов о доставке
	 * @throws NullPointerException if id, systemType, password, or addrRange is null
	 */
	public SME(String id, int priority, byte type, int typeOfNumber, int numberingPlan, int interfaceVersion, String systemType,
				  String password, String addrRange, int smeN, boolean wantAlias, boolean forceDC, int timeout, String receiptSchemeName)
			  throws NullPointerException
	{
		if (id == null || systemType == null || password == null || addrRange == null)
			throw new NullPointerException("SME System ID is null");

		this.id = id.trim();
		this.priority = priority;
		this.type = type;
		this.typeOfNumber = typeOfNumber;
		this.numberingPlan = numberingPlan;
		this.interfaceVersion = interfaceVersion;
		this.systemType = systemType;
		this.password = password;
		this.addrRange = addrRange;
		this.smeN = smeN;
		this.wantAlias = wantAlias;
		this.timeout = timeout;
		this.forceDC = forceDC;
		this.receiptSchemeName = receiptSchemeName;
		if (this.receiptSchemeName == null)
			this.receiptSchemeName = "default";
	}

	public SME(Element smeElement)
			  throws NullPointerException
	{
		this.receiptSchemeName = "default";
		this.id = smeElement.getAttribute("uid").trim();
		this.type = smeElement.getAttribute("type").equals("smpp") ? SMPP : SS7;
		NodeList list = smeElement.getElementsByTagName("param");
		for (int i = 0; i < list.getLength(); i++)
		{
			Element paramElem = (Element) list.item(i);
			String name = paramElem.getAttribute("name");
			String value = paramElem.getAttribute("value");
			if (name.equals("typeOfNumber"))
			{
				typeOfNumber = Integer.decode(value).intValue();
			}
			else if (name.equals("priority"))
			{
				priority = Integer.decode(value).intValue();
			}
			else if (name.equals("numberingPlan"))
			{
				numberingPlan = Integer.decode(value).intValue();
			}
			else if (name.equals("interfaceVersion"))
			{
				interfaceVersion = Integer.decode(value).intValue();
			}
			else if (name.equals("systemType"))
			{
				systemType = value;
			}
			else if (name.equals("password"))
			{
				password = value;
			}
			else if (name.equals("addrRange"))
			{
				addrRange = value;
			}
			else if (name.equals("smeN"))
			{
				smeN = Integer.decode(value).intValue();
			}
			else if (name.equals("wantAlias"))
			{
				wantAlias = value.equalsIgnoreCase("yes") || value.equalsIgnoreCase("true");
			}
			else if (name.equals("forceDC"))
			{
				forceDC = Boolean.valueOf(value).booleanValue();
			}
			else if (name.equals("timeout"))
			{
				timeout = Integer.decode(value).intValue();
			}
			else if (name.equals("receiptSchemeName"))
			{
				receiptSchemeName = value;
			}
		}

		if (id == null || systemType == null || password == null || addrRange == null)
			throw new NullPointerException("SME System ID is null");
	}

	public boolean equals(Object obj)
	{
		if (obj instanceof SME)
			return ((SME) obj).id.equals(id);
		else
			return super.equals(obj);
	}

	public String getAddrRange()
	{
		return addrRange;
	}

	public String getId()
	{
		return id;
	}

	public int getInterfaceVersion()
	{
		return interfaceVersion;
	}

	public int getNumberingPlan()
	{
		return numberingPlan;
	}

	public String getPassword()
	{
		return password;
	}

	public int getSmeN()
	{
		return smeN;
	}

	public String getSystemType()
	{
		return systemType;
	}

	public byte getType()
	{
		return type;
	}

	public int getTypeOfNumber()
	{
		return typeOfNumber;
	}

	public int getTimeout()
	{
		return timeout;
	}

	public boolean isWantAlias()
	{
		return wantAlias;
	}

	public int getPriority()
	{
		return priority;
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
}
