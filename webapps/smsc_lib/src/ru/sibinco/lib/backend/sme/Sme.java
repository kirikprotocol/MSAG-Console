package ru.sibinco.lib.backend.sme;

import org.apache.log4j.Category;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;

import java.io.PrintWriter;


/**
 * Created by igork
 * Date: 17.03.2004
 * Time: 17:19:54
 */
public class Sme
{
  public static final byte SMPP = 0;
  public static final byte SS7 = 1;
  public static final byte MODE_TX = 1;
  public static final byte MODE_RX = 2;
  public static final byte MODE_TRX = 3;
  public static final byte MODE_UNKNOWN = 0;

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
  private boolean disabled = false;
  private byte mode = MODE_TRX;
  private int proclimit = 0;
  private int schedlimit = 0;

  private SmeStatus status = null;

  private Category logger = Category.getInstance(this.getClass());

  /**
   * Constructor
   *
   * @param id                уникальный идентификатор SME
   * @param priority          приоритет этого SME
   * @param type              тип SME. может быть SME.SMPP или SME.SS7
   * @param typeOfNumber      тип номера SME? возможные значени€: <dl compact>
   *                          <dt>0</dt><dd>Unknown</dd>
   *                          <dt>1</dt><dd>International</dd>
   *                          <dt>2</dt><dd>National</dd>
   *                          <dt>3</dt><dd>Network Specific</dd>
   *                          <dt>4</dt><dd>Subscriber Number</dd>
   *                          <dt>5</dt><dd>Alphanumeric</dd>
   *                          <dt>6</dt><dd>Abbreviated</dd>
   *                          </dl>
   * @param numberingPlan     возможные значени€: <dl compact>
   *                          <dt>0</dt><dd>Unknown</dd>
   *                          <dt>1</dt><dd>ISDN (E163/E164)</dd>
   *                          <dt>3</dt><dd>Data (X.121)</dd>
   *                          <dt>4</dt><dd>Telex (F.69)</dd>
   *                          <dt>6</dt><dd>Land Mobile (E.212)</dd>
   *                          <dt>8</dt><dd>National</dd>
   *                          <dt>9</dt><dd>Private</dd>
   *                          <dt>10</dt><dd>ERMES</dd>
   *                          <dt>14</dt><dd>Internet (IP)</dd>
   *                          <dt>18</dt><dd>WAP Client Id (to be defined by WAP Forum)</dd>
   *                          </dl>
   * @param interfaceVersion  ¬озможные значени€: 0x34 (v3.4)
   * @param systemType        <font color=red>???</font>
   * @param password          пароль дл€ доступа SME к SMSC
   * @param addrRange         <font color=red>???</font>
   * @param smeN              <font color=red>???</font>
   * @param wantAlias
   * @param forceDC
   * @param timeout
   * @param receiptSchemeName ASCIIZ[32] с именем схемы отчетов о доставке
   * @throws NullPointerException if id, systemType, password, or addrRange is null
   */
  public Sme(String id, int priority, byte type, int typeOfNumber, int numberingPlan, int interfaceVersion, String systemType, String password, String addrRange, int smeN, boolean wantAlias, boolean forceDC, int timeout, String receiptSchemeName, boolean disabled, byte mode, int proclimit, int schedlimit) throws NullPointerException
  {
    if (id == null || systemType == null || password == null || addrRange == null)
      throw new NullPointerException("SME ID or system type or password or address range is null");

    this.id = id.trim();
    this.priority = priority;
    this.type = type;
    this.typeOfNumber = typeOfNumber;
    this.numberingPlan = numberingPlan;
    this.interfaceVersion = interfaceVersion;
    this.systemType = systemType.trim();
    this.password = password.trim();
    this.addrRange = addrRange.trim();
    this.smeN = smeN;
    this.wantAlias = wantAlias;
    this.timeout = timeout;
    this.forceDC = forceDC;
    this.receiptSchemeName = receiptSchemeName;
    this.disabled = disabled;
    this.mode = mode;
    if (this.receiptSchemeName == null)
      this.receiptSchemeName = "default";
    else
      this.receiptSchemeName = this.receiptSchemeName.trim();
    this.proclimit = proclimit;
    this.schedlimit = schedlimit;
  }

  public Sme(Element smeElement) throws NullPointerException
  {
    this.receiptSchemeName = "default";
    this.id = smeElement.getAttribute("uid").trim();
    this.type = smeElement.getAttribute("type").equals("smpp") ? SMPP : SS7;
    NodeList list = smeElement.getElementsByTagName("param");
    for (int i = 0; i < list.getLength(); i++) {
      Element paramElem = (Element) list.item(i);
      String name = paramElem.getAttribute("name");
      String value = paramElem.getAttribute("value");
      try {
        if (name.equals("typeOfNumber")) {
          typeOfNumber = Integer.decode(value).intValue();
        } else if (name.equals("priority")) {
          priority = Integer.decode(value).intValue();
        } else if (name.equals("numberingPlan")) {
          numberingPlan = Integer.decode(value).intValue();
        } else if (name.equals("interfaceVersion")) {
          interfaceVersion = Integer.decode(value).intValue();
        } else if (name.equals("systemType")) {
          systemType = value;
        } else if (name.equals("password")) {
          password = value;
        } else if (name.equals("addrRange")) {
          addrRange = value;
        } else if (name.equals("smeN")) {
          smeN = Integer.decode(value).intValue();
        } else if (name.equals("wantAlias")) {
          wantAlias = value.equalsIgnoreCase("yes") || value.equalsIgnoreCase("true");
        } else if (name.equals("forceDC")) {
          forceDC = Boolean.valueOf(value).booleanValue();
        } else if (name.equals("timeout")) {
          timeout = Integer.decode(value).intValue();
        } else if (name.equals("receiptSchemeName")) {
          receiptSchemeName = value;
        } else if (name.equals("disabled")) {
          disabled = Boolean.valueOf(value).booleanValue();
        } else if (name.equals("mode")) {
          mode = getMode(value);
        } else if (name.equals("proclimit")) {
          proclimit = Integer.decode(value).intValue();
        } else if (name.equals("schedlimit")) {
          schedlimit = Integer.decode(value).intValue();
        }
      } catch (NumberFormatException e) {
        logger.error("Int parameter \"" + name + "\" misformatted: " + value + ", skipped", e);
      }
    }

    if (id == null || systemType == null || password == null || addrRange == null)
      throw new NullPointerException("SME System ID is null");
  }

  public Sme(Sme sme)
  {
    this.id = sme.getId();
    this.priority = sme.getPriority();
    this.type = sme.getType();
    this.typeOfNumber = sme.getTypeOfNumber();
    this.numberingPlan = sme.getNumberingPlan();
    this.interfaceVersion = sme.getInterfaceVersion();
    this.systemType = sme.getSystemType();
    this.password = sme.getPassword();
    this.addrRange = sme.getAddrRange();
    this.smeN = sme.getSmeN();
    this.wantAlias = sme.isWantAlias();
    this.timeout = sme.getTimeout();
    this.forceDC = sme.isForceDC();
    this.receiptSchemeName = sme.getReceiptSchemeName();
    this.disabled = sme.isDisabled();
    this.mode = sme.getMode();
    this.receiptSchemeName = sme.getReceiptSchemeName();
    this.proclimit = sme.getProclimit();
    this.schedlimit = sme.getSchedlimit();
  }

  public PrintWriter store(PrintWriter out)
  {
    out.println("  <smerecord type=\"" + getTypeStr() + "\" uid=\"" + id + "\">");

    out.println("    <param name=\"priority\"          value=\"" + priority + "\"/>");
    out.println("    <param name=\"typeOfNumber\"      value=\"" + typeOfNumber + "\"/>");
    out.println("    <param name=\"numberingPlan\"     value=\"" + numberingPlan + "\"/>");
    out.println("    <param name=\"interfaceVersion\"  value=\"" + interfaceVersion + "\"/>");
    out.println("    <param name=\"systemType\"        value=\"" + StringEncoderDecoder.encode(systemType) + "\"/>");
    out.println("    <param name=\"password\"          value=\"" + StringEncoderDecoder.encode(password) + "\"/>");
    out.println("    <param name=\"addrRange\"         value=\"" + StringEncoderDecoder.encode(addrRange) + "\"/>");
    out.println("    <param name=\"smeN\"              value=\"" + smeN + "\"/>");
    out.println("    <param name=\"wantAlias\"         value=\"" + (wantAlias ? "yes" : "no") + "\"/>");
    out.println("    <param name=\"forceDC\"           value=\"" + forceDC + "\"/>");
    out.println("    <param name=\"timeout\"           value=\"" + timeout + "\"/>");
    out.println("    <param name=\"receiptSchemeName\" value=\"" + StringEncoderDecoder.encode(receiptSchemeName) + "\"/>");
    out.println("    <param name=\"disabled\"          value=\"" + disabled + "\"/>");
    out.println("    <param name=\"mode\"              value=\"" + getModeStr() + "\"/>");
    if (proclimit > 0)
      out.println("    <param name=\"proclimit\"         value=\"" + proclimit + "\"/>");
    if (schedlimit > 0)
      out.println("    <param name=\"schedlimit\"        value=\"" + schedlimit + "\"/>");


    out.println("  </smerecord>");
    return out;
  }

  private String getTypeStr()
  {
    switch (type) {
      case SMPP:
        return "smpp";
      case SS7:
        return "ss7";
      default:
        return "unknown";
    }
  }

  public boolean equals(Object obj)
  {
    if (obj instanceof Sme)
      return ((Sme) obj).id.equals(id);
    else
      return super.equals(obj);
  }

  public int hashCode()
  {
    return id.hashCode();
  }

  public String getModeStr()
  {
    switch (mode) {
      case MODE_TX:
        return "tx";
      case MODE_RX:
        return "rx";
      case MODE_TRX:
        return "trx";
      default:
        return "unknown";
    }
  }

  private static byte getMode(String modeStr)
  {
    if ("tx".equalsIgnoreCase(modeStr))
      return MODE_TX;
    else if ("rx".equalsIgnoreCase(modeStr))
      return MODE_RX;
    else if ("trx".equalsIgnoreCase(modeStr))
      return MODE_TRX;
    return 0;
  }

  public void update(Sme newSme)
  {
    id = newSme.getId();
    priority = newSme.getPriority();
    type = newSme.getType();
    typeOfNumber = newSme.getTypeOfNumber();
    numberingPlan = newSme.getNumberingPlan();
    interfaceVersion = newSme.getInterfaceVersion();
    systemType = newSme.getSystemType();
    password = newSme.getPassword();
    addrRange = newSme.getAddrRange();
    smeN = newSme.getSmeN();
    wantAlias = newSme.isWantAlias();
    timeout = newSme.getTimeout();
    forceDC = newSme.isForceDC();
    receiptSchemeName = newSme.getReceiptSchemeName();
    disabled = newSme.isDisabled();
    mode = newSme.getMode();
    proclimit = newSme.getProclimit();
    schedlimit = newSme.getSchedlimit();
  }

  public String getId()
  {
    return id;
  }

  public void setId(String id)
  {
    this.id = id;
  }

  public int getPriority()
  {
    return priority;
  }

  public void setPriority(int priority)
  {
    this.priority = priority;
  }

  public byte getType()
  {
    return type;
  }

  public void setType(byte type)
  {
    this.type = type;
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

  public int getInterfaceVersion()
  {
    return interfaceVersion;
  }

  public void setInterfaceVersion(int interfaceVersion)
  {
    this.interfaceVersion = interfaceVersion;
  }

  public String getSystemType()
  {
    return systemType;
  }

  public void setSystemType(String systemType)
  {
    this.systemType = systemType;
  }

  public String getPassword()
  {
    return password;
  }

  public void setPassword(String password)
  {
    this.password = password;
  }

  public String getAddrRange()
  {
    return addrRange;
  }

  public void setAddrRange(String addrRange)
  {
    this.addrRange = addrRange;
  }

  public int getSmeN()
  {
    return smeN;
  }

  public void setSmeN(int smeN)
  {
    this.smeN = smeN;
  }

  public boolean isWantAlias()
  {
    return wantAlias;
  }

  public void setWantAlias(boolean wantAlias)
  {
    this.wantAlias = wantAlias;
  }

  public int getTimeout()
  {
    return timeout;
  }

  public void setTimeout(int timeout)
  {
    this.timeout = timeout;
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

  public byte getMode()
  {
    return mode;
  }

  public void setMode(byte mode)
  {
    this.mode = mode;
  }

  public int getProclimit()
  {
    return proclimit;
  }

  public void setProclimit(int proclimit)
  {
    this.proclimit = proclimit;
  }

  public int getSchedlimit()
  {
    return schedlimit;
  }

  public void setSchedlimit(int schedlimit)
  {
    this.schedlimit = schedlimit;
  }

  public SmeStatus getStatus()
  {
    return status;
  }

  public void setStatus(SmeStatus status)
  {
    this.status = status;
  }
}
