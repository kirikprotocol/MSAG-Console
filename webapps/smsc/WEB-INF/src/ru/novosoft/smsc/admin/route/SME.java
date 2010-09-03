package ru.novosoft.smsc.admin.route;

/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 13:50:54
 */

import org.apache.log4j.Category;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import java.io.PrintWriter;
import java.util.StringTokenizer;


public class SME {
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
//    private int interfaceVersion = 0;
    private String systemType = null;
    private String password = null;
    private String addrRange = null;
    private int smeN = 0;
    private boolean wantAlias = false;
    private int timeout = 0;
//    private boolean forceDC = false;
    private String receiptSchemeName = null;
    private boolean disabled = false;
    private byte mode = MODE_TRX;
    private int proclimit = 0;
    private int schedlimit = 0;
    private int accessMask = 1; // for closed groups

    private boolean carryOrgDescriptor;
    private boolean carryOrgUserInfo;
    private boolean carrySccpInfo;
    private boolean fillExtraDescriptor; // for Extra mode
    private boolean forceSmeReceipt;
    private boolean forceGsmDataCoding;
    private boolean smppPlus;
    private boolean defaultDcLatin1;


    private static final Category logger = Category.getInstance(SME.class);

    public SME(String id) {
      this.id = id;
    }

//    public SME(String id, int priority, byte type, int typeOfNumber, int numberingPlan, int interfaceVersion, String systemType, String password, String addrRange, int smeN, boolean wantAlias, boolean forceDC, int timeout, String receiptSchemeName, boolean disabled, byte mode, int proclimit, int schedlimit, int accessMask) throws NullPointerException {
//        if (id == null || systemType == null || password == null || addrRange == null)
//            throw new NullPointerException("SME System ID is null");
//
//        this.id = id.trim();
//        this.priority = priority;
//        this.type = type;
//        this.typeOfNumber = typeOfNumber;
//        this.numberingPlan = numberingPlan;
//        this.interfaceVersion = interfaceVersion;
//        this.systemType = systemType.trim();
//        this.password = password.trim();
//        this.addrRange = addrRange.trim();
//        this.smeN = smeN;
//        this.wantAlias = wantAlias;
//        this.timeout = timeout;
//        this.forceDC = forceDC;
//        this.receiptSchemeName = receiptSchemeName;
//        this.disabled = disabled;
//        this.mode = mode;
//        if (this.receiptSchemeName == null)
//            this.receiptSchemeName = "default";
//        else
//            this.receiptSchemeName = this.receiptSchemeName.trim();
//        this.proclimit = proclimit;
//        this.schedlimit = schedlimit;
//        this.accessMask = accessMask;
//    }

    public SME(Element smeElement) throws NullPointerException {
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
                    int interfaceVersion = Integer.decode(value).intValue();
                    setInterfaceVersion(interfaceVersion);
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
                    forceGsmDataCoding = Boolean.valueOf(value).booleanValue();
                } else if (name.equals("timeout")) {
                    timeout = Integer.decode(value).intValue();
                } else if (name.equals("receiptSchemeName")) {
                    receiptSchemeName = value;
                } else if (name.equals("disabled")) {
                    disabled = Boolean.valueOf(value).booleanValue();
                } else if (name.equals("mode")) {
                    mode = convertModeStr(value);
                } else if (name.equals("proclimit")) {
                    proclimit = Integer.decode(value).intValue();
                } else if (name.equals("schedlimit")) {
                    schedlimit = Integer.decode(value).intValue();
                } else if (name.equals("accessMask")) {
                    accessMask = Integer.decode(value).intValue();
                } else if (name.equals("flags")) {
                    StringTokenizer flags = new StringTokenizer(value,",");
                    while (flags.hasMoreTokens()) {
                      String flag = flags.nextToken().trim();
                      if (flag.equals("carryOrgDescriptor"))
                        carryOrgDescriptor = true;
                      else if (flag.equals("carryOrgUserInfo"))
                        carryOrgUserInfo = true;
                      else if (flag.equals("carrySccpInfo"))
                        carrySccpInfo = true;
                      else if (flag.equals("fillExtraDescriptor"))
                        fillExtraDescriptor = true;
                      else if (flag.equals("forceSmeReceipt"))
                        forceSmeReceipt = true;
                      else if (flag.equals("forceGsmDataCoding"))
                        forceGsmDataCoding = true;
                      else if (flag.equals("smppPlus"))
                        smppPlus = true;
                      else if (flag.equals("defaultDcLatin1"))
                        defaultDcLatin1 = true;
                      else
                        logger.error("Unknown SME flag: " + flag + ". It will be skipped.");
                    }
                }
            } catch (NumberFormatException e) {
                logger.error("Int parameter \"" + name + "\" misformatted: " + value + ", skipped", e);
            }
        }

        if (id == null || systemType == null || password == null || addrRange == null)
            throw new NullPointerException("SME System ID is null");
    }

    public PrintWriter store(PrintWriter out) {
        out.println("  <smerecord type=\"" + getTypeStr() + "\" uid=\"" + id + "\">");

        out.println("    <param name=\"priority\"          value=\"" + priority + "\"/>");
        out.println("    <param name=\"typeOfNumber\"      value=\"" + typeOfNumber + "\"/>");
        out.println("    <param name=\"numberingPlan\"     value=\"" + numberingPlan + "\"/>");
//        out.println("    <param name=\"interfaceVersion\"  value=\"" + interfaceVersion + "\"/>");
        out.println("    <param name=\"systemType\"        value=\"" + StringEncoderDecoder.encode(systemType) + "\"/>");
        out.println("    <param name=\"password\"          value=\"" + StringEncoderDecoder.encode(password) + "\"/>");
        out.println("    <param name=\"addrRange\"         value=\"" + StringEncoderDecoder.encode(addrRange) + "\"/>");
        out.println("    <param name=\"smeN\"              value=\"" + smeN + "\"/>");
        out.println("    <param name=\"wantAlias\"         value=\"" + (wantAlias ? "yes" : "no") + "\"/>");
//        out.println("    <param name=\"forceDC\"           value=\"" + forceDC + "\"/>");
        out.println("    <param name=\"timeout\"           value=\"" + timeout + "\"/>");
        out.println("    <param name=\"receiptSchemeName\" value=\"" + StringEncoderDecoder.encode(receiptSchemeName) + "\"/>");
        out.println("    <param name=\"disabled\"          value=\"" + disabled + "\"/>");
        out.println("    <param name=\"mode\"              value=\"" + getModeStr() + "\"/>");
        if (proclimit > 0)
            out.println("    <param name=\"proclimit\"         value=\"" + proclimit + "\"/>");
        if (schedlimit > 0)
            out.println("    <param name=\"schedlimit\"        value=\"" + schedlimit + "\"/>");
        if (accessMask > 0)
            out.println("    <param name=\"accessMask\"        value=\"" + accessMask + "\"/>");
        out.println("    <param name=\"flags\"              value=\"" + getFlagsStr() + "\"/>");
        out.println("  </smerecord>");
        return out;
    }

    public String getFlagsStr() {
      StringBuffer flags = new StringBuffer();
      if (carryOrgDescriptor)
          addFlag(flags, "carryOrgDescriptor");
      if (carryOrgUserInfo)
          addFlag(flags, "carryOrgUserInfo");
      if (carrySccpInfo)
          addFlag(flags, "carrySccpInfo");
      if (fillExtraDescriptor)
          addFlag(flags, "fillExtraDescriptor");
      if (forceSmeReceipt)
          addFlag(flags, "forceSmeReceipt");
      if (forceGsmDataCoding)
          addFlag(flags, "forceGsmDataCoding");
      if (smppPlus)
          addFlag(flags, "smppPlus");
      if (defaultDcLatin1)
          addFlag(flags, "defaultDcLatin1");
      return flags.toString();
    }

    private void addFlag(StringBuffer sb, String flag) {
      if (sb.length() > 0)
        sb.append(',');
      sb.append(flag);
    }

    private String getTypeStr() {
        switch (type) {
            case SMPP:
                return "smpp";
            case SS7:
                return "ss7";
            default:
                return "unknown";
        }
    }

    public boolean equals(Object obj) {
        if (obj instanceof SME)
            return ((SME) obj).id.equals(id);
        else
            return super.equals(obj);
    }

    public String getAddrRange() {
        return addrRange;
    }

    public String getId() {
        return id;
    }

//    public int getInterfaceVersion() {
//        return interfaceVersion;
//    }

    public int getNumberingPlan() {
        return numberingPlan;
    }

    public String getPassword() {
        return password;
    }

    public int getSmeN() {
        return smeN;
    }

    public String getSystemType() {
        return systemType;
    }

    public byte getType() {
        return type;
    }

    public int getTypeOfNumber() {
        return typeOfNumber;
    }

    public int getTimeout() {
        return timeout;
    }

    public boolean isWantAlias() {
        return wantAlias;
    }

    public int getPriority() {
        return priority;
    }

//    public boolean isForceDC() {
//        return forceDC;
//    }

  public boolean isCarryOrgDescriptor() {
    return carryOrgDescriptor;
  }

  public boolean isCarryOrgUserInfo() {
    return carryOrgUserInfo;
  }

  public boolean isCarrySccpInfo() {
    return carrySccpInfo;
  }

  public boolean isFillExtraDescriptor() {
    return fillExtraDescriptor;
  }

  public boolean isForceSmeReceipt() {
    return forceSmeReceipt;
  }

  public boolean isForceGsmDataCoding() {
    return forceGsmDataCoding;
  }

  public boolean isSmppPlus() {
    return smppPlus;
  }

  public boolean isDefaultDcLatin1() {
    return defaultDcLatin1;
  }

  public String getReceiptSchemeName() {
        return receiptSchemeName;
    }

    public boolean isDisabled() {
        return disabled;
    }

    public byte getMode() {
        return mode;
    }

    public String getModeStr() {
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

    private static byte convertModeStr(String modeStr) {
        if ("tx".equalsIgnoreCase(modeStr))
            return MODE_TX;
        else if ("rx".equalsIgnoreCase(modeStr))
            return MODE_RX;
        else if ("trx".equalsIgnoreCase(modeStr))
            return MODE_TRX;
        return 0;
    }

    public void update(SME newSme) {
        id = newSme.getId();
        priority = newSme.getPriority();
        type = newSme.getType();
        typeOfNumber = newSme.getTypeOfNumber();
        numberingPlan = newSme.getNumberingPlan();
//        interfaceVersion = newSme.getInterfaceVersion();
        systemType = newSme.getSystemType();
        password = newSme.getPassword();
        addrRange = newSme.getAddrRange();
        smeN = newSme.getSmeN();
        wantAlias = newSme.isWantAlias();
        timeout = newSme.getTimeout();
//        forceDC = newSme.isForceDC();
        receiptSchemeName = newSme.getReceiptSchemeName();
        disabled = newSme.isDisabled();
        mode = newSme.getMode();
        proclimit = newSme.getProclimit();
        schedlimit = newSme.getSchedlimit();
        accessMask = newSme.getAccessMask();

        carryOrgDescriptor = newSme.isCarryOrgDescriptor();
        carryOrgUserInfo = newSme.isCarryOrgUserInfo();
        carrySccpInfo = newSme.isCarrySccpInfo();
        fillExtraDescriptor = newSme.isFillExtraDescriptor();
        forceSmeReceipt = newSme.isForceSmeReceipt();
        forceGsmDataCoding = newSme.isForceGsmDataCoding();
        smppPlus = newSme.isSmppPlus();
        defaultDcLatin1 = newSme.isDefaultDcLatin1();
    }

    public int getProclimit() {
        return proclimit;
    }

    public int getSchedlimit() {
        return schedlimit;
    }

    public int getAccessMask() {
        return accessMask;
    }

  public void setId(String id) {
    this.id = id;
  }

  public void setPriority(int priority) {
    this.priority = priority;
  }

  public void setType(byte type) {
    this.type = type;
  }

  public void setTypeOfNumber(int typeOfNumber) {
    this.typeOfNumber = typeOfNumber;
  }

  public void setNumberingPlan(int numberingPlan) {
    this.numberingPlan = numberingPlan;
  }

  public void setSystemType(String systemType) {
    this.systemType = systemType;
  }

  public void setPassword(String password) {
    this.password = password;
  }

  public void setAddrRange(String addrRange) {
    this.addrRange = addrRange;
  }

  public void setSmeN(int smeN) {
    this.smeN = smeN;
  }

  public void setWantAlias(boolean wantAlias) {
    this.wantAlias = wantAlias;
  }

  public void setTimeout(int timeout) {
    this.timeout = timeout;
  }

  public void setReceiptSchemeName(String receiptSchemeName) {
    this.receiptSchemeName = receiptSchemeName;
  }

  public void setDisabled(boolean disabled) {
    this.disabled = disabled;
  }

  public void setMode(byte mode) {
    this.mode = mode;
  }

  public void setProclimit(int proclimit) {
    this.proclimit = proclimit;
  }

  public void setSchedlimit(int schedlimit) {
    this.schedlimit = schedlimit;
  }

  public void setAccessMask(int accessMask) {
    this.accessMask = accessMask;
  }

  public void setCarryOrgDescriptor(boolean carryOrgDescriptor) {
    this.carryOrgDescriptor = carryOrgDescriptor;
  }

  public void setCarryOrgUserInfo(boolean carryOrgUserInfo) {
    this.carryOrgUserInfo = carryOrgUserInfo;
  }

  public void setCarrySccpInfo(boolean carrySccpInfo) {
    this.carrySccpInfo = carrySccpInfo;
  }

  public void setFillExtraDescriptor(boolean fillExtraDescriptor) {
    this.fillExtraDescriptor = fillExtraDescriptor;
  }

  public void setForceSmeReceipt(boolean forceSmeReceipt) {
    this.forceSmeReceipt = forceSmeReceipt;
  }

  public void setForceGsmDataCoding(boolean forceGsmDataCoding) {
    this.forceGsmDataCoding = forceGsmDataCoding;
  }

  public void setSmppPlus(boolean smppPlus) {
    this.smppPlus = smppPlus;
  }

  public void setDefaultDcLatin1(boolean defaultDcLatin1) {
    this.defaultDcLatin1 = defaultDcLatin1;
  }

  public void setInterfaceVersion(int interfaceVersion) {
    if ((interfaceVersion >> 4) == 5)
      forceSmeReceipt = true;
    if ((interfaceVersion & 0xF) == 9) {
      carryOrgDescriptor = true;
      carryOrgUserInfo = true;
      carrySccpInfo = true;
      fillExtraDescriptor = true;
    }
  }

  public void setForceDC(boolean forceDC) {
    forceGsmDataCoding = forceDC;
  }
}
