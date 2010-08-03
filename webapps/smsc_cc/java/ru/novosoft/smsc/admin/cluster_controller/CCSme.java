package ru.novosoft.smsc.admin.cluster_controller;

import mobi.eyeline.protogen.framework.InvalidEnumValueException;
import ru.novosoft.smsc.admin.cluster_controller.protocol.SmeBindMode;
import ru.novosoft.smsc.admin.cluster_controller.protocol.SmeParams;

/**
 * @author Artem Snopkov
 */
public class CCSme {

  public static final int CONNECT_TYPE_LOAD_BALANCER = 1;
  public static final int CONNECT_TYPE_DIRECT = 2;

  public static final int BIND_MODE_TX=0;
  public static final int BIND_MODE_RX=1;
  public static final int BIND_MODE_TRX=2;
  public static final int BIND_MODE_UNKNOWN=3;

  public static final int SME_STATUS_DISCONNECTED=0;
  public static final int SME_STATUS_BOUND=1;
  public static final int SME_STATUS_INTERNAL=2;
  public static final int SME_STATUS_UNKNOWN=4;

  private String id;
  private int priority;
  private String systemType;
  private String password;
  private String addrRange;
  private int smeN;
  private boolean wantAlias;
  private int timeout;
  private String receiptScheme;
  private boolean disabled;
  private int mode;
  private int procLimit;
  private int schedLimit;
  private int accessMask;
  boolean flagCarryOrgDescriptor;
  boolean flagCarryOrgAbonentInfo;
  boolean flagCarrySccpInfo;
  boolean flagFillExtraDescriptor;
  boolean flagForceReceiptToSme;
  boolean flagForceGsmDatacoding;
  boolean flagSmppPlus;

  public String getId() {
    return id;
  }

  public void setId(String id) {
    this.id = id;
  }

  public int getPriority() {
    return priority;
  }

  public void setPriority(int priority) {
    this.priority = priority;
  }

  public String getSystemType() {
    return systemType;
  }

  public void setSystemType(String systemType) {
    this.systemType = systemType;
  }

  public String getPassword() {
    return password;
  }

  public void setPassword(String password) {
    this.password = password;
  }

  public String getAddrRange() {
    return addrRange;
  }

  public void setAddrRange(String addrRange) {
    this.addrRange = addrRange;
  }

  public int getSmeN() {
    return smeN;
  }

  public void setSmeN(int smeN) {
    this.smeN = smeN;
  }

  public boolean isWantAlias() {
    return wantAlias;
  }

  public void setWantAlias(boolean wantAlias) {
    this.wantAlias = wantAlias;
  }

  public int getTimeout() {
    return timeout;
  }

  public void setTimeout(int timeout) {
    this.timeout = timeout;
  }

  public String getReceiptScheme() {
    return receiptScheme;
  }

  public void setReceiptScheme(String receiptScheme) {
    this.receiptScheme = receiptScheme;
  }

  public boolean isDisabled() {
    return disabled;
  }

  public void setDisabled(boolean disabled) {
    this.disabled = disabled;
  }

  public int getMode() {
    return mode;
  }

  public void setMode(int mode) {
    this.mode = mode;
  }

  public int getProcLimit() {
    return procLimit;
  }

  public void setProcLimit(int procLimit) {
    this.procLimit = procLimit;
  }

  public int getSchedLimit() {
    return schedLimit;
  }

  public void setSchedLimit(int schedLimit) {
    this.schedLimit = schedLimit;
  }

  public int getAccessMask() {
    return accessMask;
  }

  public void setAccessMask(int accessMask) {
    this.accessMask = accessMask;
  }

  public boolean isFlagCarryOrgDescriptor() {
    return flagCarryOrgDescriptor;
  }

  public void setFlagCarryOrgDescriptor(boolean flagCarryOrgDescriptor) {
    this.flagCarryOrgDescriptor = flagCarryOrgDescriptor;
  }

  public boolean isFlagCarryOrgAbonentInfo() {
    return flagCarryOrgAbonentInfo;
  }

  public void setFlagCarryOrgAbonentInfo(boolean flagCarryOrgAbonentInfo) {
    this.flagCarryOrgAbonentInfo = flagCarryOrgAbonentInfo;
  }

  public boolean isFlagCarrySccpInfo() {
    return flagCarrySccpInfo;
  }

  public void setFlagCarrySccpInfo(boolean flagCarrySccpInfo) {
    this.flagCarrySccpInfo = flagCarrySccpInfo;
  }

  public boolean isFlagFillExtraDescriptor() {
    return flagFillExtraDescriptor;
  }

  public void setFlagFillExtraDescriptor(boolean flagFillExtraDescriptor) {
    this.flagFillExtraDescriptor = flagFillExtraDescriptor;
  }

  public boolean isFlagForceReceiptToSme() {
    return flagForceReceiptToSme;
  }

  public void setFlagForceReceiptToSme(boolean flagForceReceiptToSme) {
    this.flagForceReceiptToSme = flagForceReceiptToSme;
  }

  public boolean isFlagForceGsmDatacoding() {
    return flagForceGsmDatacoding;
  }

  public void setFlagForceGsmDatacoding(boolean flagForceGsmDatacoding) {
    this.flagForceGsmDatacoding = flagForceGsmDatacoding;
  }

  public boolean isFlagSmppPlus() {
    return flagSmppPlus;
  }

  public void setFlagSmppPlus(boolean flagSmppPlus) {
    this.flagSmppPlus = flagSmppPlus;
  }

  SmeParams toSmeParams() {
    SmeParams p = new SmeParams();
    p.setAccessMask(accessMask);
    p.setAddrRange(addrRange);
    p.setDisabled(disabled);
    p.setId(id);
    try {
      p.setMode(SmeBindMode.valueOf((byte)mode));
    } catch (InvalidEnumValueException e) {
      p.setMode(SmeBindMode.modeTrx);
    }

    p.setPassword(password);
    p.setPriority(priority);
    p.setProcLimit(procLimit);
    p.setReceiptScheme(receiptScheme);
    p.setSchedLimit(schedLimit);
    p.setSmeN(smeN);
    p.setSystemType(systemType);
    p.setTimeout(timeout);
    p.setWantAlias(wantAlias);
    p.setFlagCarryOrgAbonentInfo(flagCarryOrgAbonentInfo);
    p.setFlagCarryOrgDescriptor(flagCarryOrgDescriptor);
    p.setFlagCarrySccpInfo(flagCarrySccpInfo);
    p.setFlagFillExtraDescriptor(flagFillExtraDescriptor);
    p.setFlagForceGsmDatacoding(flagForceGsmDatacoding);
    p.setFlagForceReceiptToSme(flagForceReceiptToSme);
    p.setFlagSmppPlus(flagSmppPlus);
    return p;
  }
}
