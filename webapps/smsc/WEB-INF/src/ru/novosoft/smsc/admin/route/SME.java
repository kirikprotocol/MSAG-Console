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
  private byte type = SMPP;
  private int typeOfNumber = 0;
  private int numberingPlan = 0;
  private int interfaceVersion = 0;
  private String systemType = null;
  private String password = null;
  private String addrRange = null;
  private int smeN = 0;

  public SME(String id, byte type, int typeOfNumber, int numberingPlan, int interfaceVersion, String systemType,
             String password, String addrRange, int smeN)
          throws NullPointerException
  {
    if (id == null || systemType == null || password == null || addrRange == null)
      throw new NullPointerException("SME System ID is null");

    this.id = id.trim();
    this.type = type;
    this.typeOfNumber = typeOfNumber;
    this.numberingPlan = numberingPlan;
    this.interfaceVersion = interfaceVersion;
    this.systemType = systemType;
    this.password = password;
    this.addrRange = addrRange;
    this.smeN = smeN;
  }

  public SME(Element smeElement)
          throws NullPointerException
  {
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
    }

    if (id == null || systemType == null || password == null || addrRange == null)
      throw new NullPointerException("SME System ID is null");
  }

  private void init(String id, byte type, int typeOfNumber, int numberingPlan,
                    int interfaceVersion, String systemType, String password,
                    String addrRange, int smeN)
          throws NullPointerException
  {
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
}
