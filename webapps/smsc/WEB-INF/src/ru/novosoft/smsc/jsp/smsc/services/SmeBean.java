package ru.novosoft.smsc.jsp.smsc.services;

import ru.novosoft.smsc.admin.route.SME;
import ru.novosoft.smsc.jsp.PageBean;

/**
 * Created by igork
 * Date: Feb 19, 2003
 * Time: 6:04:35 PM
 */
public class SmeBean extends PageBean {
    protected String serviceId = null;
    protected int priority = 0;
    protected String systemType = "";
    protected int typeOfNumber = 0;
    protected int numberingPlan = 0;
    protected String rangeOfAddress = "";
    protected String password = "";
    protected int timeout = 8;
    protected boolean wantAlias = false;
    protected String receiptSchemeName = "default";
    protected boolean disabled = false;
    protected byte mode = SME.MODE_TRX;
    protected int proclimit = 0;
    protected int schedlimit = 0;

    protected int accessMask = 1;
    protected int extraFlag = -1;

    protected boolean carryOrgDescriptor;
    protected boolean carryOrgUserInfo;
    protected boolean carrySccpInfo;
    protected boolean fillExtraDescriptor; // for Extra mode
    protected boolean forceSmeReceipt;
    protected boolean forceGsmDataCoding;
    protected boolean smppPlus;
    protected boolean defaultDcLatin1;


    protected int convertInterfaceVersion(String version) {
        int pos = version.indexOf('.');
        if (pos > 0) {
            return (Integer.parseInt(version.substring(0, pos)) << 4) + (Integer.parseInt(version.substring(pos + 1)));
        } else
            return -1;
    }

    protected void cleanup() {
        serviceId = null;
        priority = 0;
        systemType = "";
        typeOfNumber = 0;
        numberingPlan = 0;
        rangeOfAddress = "";
        password = "";
        timeout = 8;
        wantAlias = false;
        receiptSchemeName = "default";
        disabled = false;
        mode = SME.MODE_TRX;
        proclimit = 0;
        schedlimit = 0;
        accessMask = 1;
        carryOrgDescriptor = false;
        carryOrgUserInfo = false;
        carrySccpInfo = false;
        fillExtraDescriptor = false;
        forceSmeReceipt = false;
        forceGsmDataCoding = false;
        smppPlus = false;
        defaultDcLatin1 = false;
    }

    protected SME getSME() {
      SME sme = new SME(serviceId);
      sme.setPriority(priority);
      sme.setType(SME.SMPP);
      sme.setTypeOfNumber(typeOfNumber);
      sme.setNumberingPlan(numberingPlan);
      sme.setSystemType(systemType);
      sme.setPassword(password);
      sme.setAddrRange(rangeOfAddress);
      sme.setSmeN(extraFlag);
      sme.setWantAlias(wantAlias);
      sme.setTimeout(timeout);
      sme.setReceiptSchemeName(receiptSchemeName);
      sme.setDisabled(disabled);
      sme.setMode(mode);
      sme.setProclimit(proclimit);
      sme.setSchedlimit(schedlimit);
      sme.setAccessMask(accessMask);
      sme.setCarryOrgDescriptor(carryOrgDescriptor);
      sme.setCarryOrgUserInfo(carryOrgUserInfo);
      sme.setCarrySccpInfo(carrySccpInfo);
      sme.setFillExtraDescriptor(fillExtraDescriptor);
      sme.setForceSmeReceipt(forceSmeReceipt);
      sme.setForceGsmDataCoding(forceGsmDataCoding);
      sme.setSmppPlus(smppPlus);
      sme.setDefaultDcLatin1(defaultDcLatin1);
      return sme;
    }

    /**
     * **************************** properties ***************************************
     */
    public String getServiceId() {
        return serviceId;
    }

    public void setServiceId(String serviceId) {
        this.serviceId = serviceId.trim();
    }

    public String getPriority() {
        return Integer.toString(priority);
    }

    public void setPriority(String priority) {
        try {
            this.priority = Integer.decode(priority).intValue();
        } catch (NumberFormatException e) {
            this.priority = 0;
        }
    }

    public String getSystemType() {
        return systemType;
    }

    public void setSystemType(String systemType) {
        this.systemType = systemType.trim();
    }

    public int getTypeOfNumberInt() {
        return typeOfNumber;
    }

    public String getTypeOfNumber() {
        return Integer.toString(typeOfNumber);
    }

    public void setTypeOfNumber(String typeOfNumber) {
        try {
            this.typeOfNumber = Integer.decode(typeOfNumber).intValue();
        } catch (NumberFormatException e) {
            this.typeOfNumber = 0;
        }
    }

    public int getNumberingPlanInt() {
        return numberingPlan;
    }

    public String getNumberingPlan() {
        return Integer.toString(numberingPlan);
    }

    public void setNumberingPlan(String numberingPlan) {
        try {
            this.numberingPlan = Integer.decode(numberingPlan).intValue();
        } catch (NumberFormatException e) {
            this.numberingPlan = 0;
        }
    }

    public String getRangeOfAddress() {
        return rangeOfAddress;
    }

    public void setRangeOfAddress(String rangeOfAddress) {
        this.rangeOfAddress = rangeOfAddress.trim();
    }

    public String getPassword() {
        return password;
    }

    public void setPassword(String password) {
        this.password = password.trim();
    }

    public String getAccessMask() {
        return Integer.toString(accessMask);
    }

    public void setAccessMask(String accessMask) {
        try {
            this.accessMask = Integer.decode(accessMask).intValue();
        } catch (NumberFormatException e) {
            this.accessMask = 1;
        }
    }

    public String getTimeout() {
        return Integer.toString(timeout);
    }

    public void setTimeout(String timeout) {
        try {
            this.timeout = Integer.decode(timeout).intValue();
        } catch (NumberFormatException e) {
            this.timeout = 8;
        }
    }

    public boolean isWantAlias() {
        return wantAlias;
    }

    public void setWantAlias(boolean wantAlias) {
        this.wantAlias = wantAlias;
    }

    public String getReceiptSchemeName() {
        return receiptSchemeName;
    }

    public void setReceiptSchemeName(String receiptSchemeName) {
        this.receiptSchemeName = receiptSchemeName.trim();
    }

    public boolean isDisabled() {
        return disabled;
    }

    public void setDisabled(boolean disabled) {
        this.disabled = disabled;
    }

    public byte getModeByte() {
        return mode;
    }

    public String getMode() {
        return Byte.toString(mode);
    }

    public void setMode(String mode) {
        try {
            this.mode = Byte.decode(mode).byteValue();
        } catch (NumberFormatException e) {
            this.mode = 0;
        }
    }

    public int getProclimitInt() {
        return proclimit;
    }

    public void setProclimitInt(int proclimit) {
        this.proclimit = proclimit;
    }

    public int getSchedlimitInt() {
        return schedlimit;
    }

    public void setSchedlimitInt(int schedlimit) {
        this.schedlimit = schedlimit;
    }

    public String getProclimit() {
        return String.valueOf(proclimit);
    }

    public void setProclimit(String proclimit) {
        try {
            this.proclimit = Integer.decode(proclimit).intValue();
        } catch (NumberFormatException e) {
            logger.error("proclimit int parameter misformatted: \"" + proclimit + "\", skipped");
            this.proclimit = 0;
        }
        ;
    }

    public String getSchedlimit() {
        return String.valueOf(schedlimit);
    }

    public void setSchedlimit(String schedlimit) {
        try {
            this.schedlimit = Integer.decode(schedlimit).intValue();
        } catch (NumberFormatException e) {
            logger.error("schedlimit int parameter misformatted: \"" + schedlimit + "\", skipped");
            this.schedlimit = 0;
        }
    }

    public String getExtraFlag() {
        return String.valueOf(extraFlag);
    }

    public void setExtraFlag(String extraFlag) {
      try {
          this.extraFlag = Integer.parseInt(extraFlag);
      } catch (NumberFormatException e) {
          logger.error("extraFlag int parameter misformatted: \"" + extraFlag + "\", skipped");
          this.extraFlag = -1;
      }
    }

  public boolean isCarryOrgDescriptor() {
    return carryOrgDescriptor;
  }

  public void setCarryOrgDescriptor(boolean carryOrgDescriptor) {
    this.carryOrgDescriptor = carryOrgDescriptor;
  }

  public boolean isCarryOrgUserInfo() {
    return carryOrgUserInfo;
  }

  public void setCarryOrgUserInfo(boolean carryOrgUserInfo) {
    this.carryOrgUserInfo = carryOrgUserInfo;
  }

  public boolean isCarrySccpInfo() {
    return carrySccpInfo;
  }

  public void setCarrySccpInfo(boolean carrySccpInfo) {
    this.carrySccpInfo = carrySccpInfo;
  }

  public boolean isFillExtraDescriptor() {
    return fillExtraDescriptor;
  }

  public void setFillExtraDescriptor(boolean fillExtraDescriptor) {
    this.fillExtraDescriptor = fillExtraDescriptor;
  }

  public boolean isForceSmeReceipt() {
    return forceSmeReceipt;
  }

  public void setForceSmeReceipt(boolean forceSmeReceipt) {
    this.forceSmeReceipt = forceSmeReceipt;
  }

  public boolean isForceGsmDataCoding() {
    return forceGsmDataCoding;
  }

  public void setForceGsmDataCoding(boolean forceGsmDataCoding) {
    this.forceGsmDataCoding = forceGsmDataCoding;
  }

  public boolean isSmppPlus() {
    return smppPlus;
  }

  public void setSmppPlus(boolean smppPlus) {
    this.smppPlus = smppPlus;
  }

  public boolean isDefaultDcLatin1() {
    return defaultDcLatin1;
  }

  public void setDefaultDcLatin1(boolean defaultDcLatin1) {
    this.defaultDcLatin1 = defaultDcLatin1;
  }
}
