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

  public static final int SME_STATUS_DISCONNECTED=1;
  public static final int SME_STATUS_BOUND=2;
  public static final int SME_STATUS_INTERNAL=3;
  public static final int SME_STATUS_UNKNOWN=4;


  private final SmeParams sme; 

  public CCSme() {
    this.sme = new SmeParams();
    sme.setFlagCarryOrgDescriptor(false);
    sme.setFlagCarrySccpInfo(false);
    sme.setFlagSmppPlus(false);
    sme.setFlagCarryOrgAbonentInfo(false);
    sme.setFlagFillExtraDescriptor(false);
    sme.setFlagForceGsmDatacoding(false);
    sme.setFlagForceReceiptToSme(false);
    sme.setAccessMask(0);
    sme.setAddrRange("");
    sme.setDisabled(false);
    sme.setId(null);
    sme.setMode(SmeBindMode.modeTrx);
    sme.setPassword(null);
    sme.setPriority(0);
    sme.setProcLimit(0);
    sme.setReceiptScheme(null);
    sme.setSchedLimit(0);
    sme.setSmeN(0);
    sme.setSystemType(null);
    sme.setTimeout(0);
    sme.setWantAlias(false);
  }

  public String getId() {
    return sme.getId();
  }

  public void setId(String id) {
    sme.setId(id);
  }

  public int getPriority() {
    return sme.getPriority();
  }

  public void setPriority(int priority) {
    sme.setPriority(priority);
  }

  public String getSystemType() {
    return sme.getSystemType();
  }

  public void setSystemType(String systemType) {
    sme.setSystemType(systemType);
  }

  public String getPassword() {
    return sme.getPassword();
  }

  public void setPassword(String password) {
    sme.setPassword(password);
  }

  public String getAddrRange() {
    return sme.getAddrRange();
  }

  public void setAddrRange(String addrRange) {
    sme.setAddrRange(addrRange);
  }

  public int getSmeN() {
    return sme.getSmeN();
  }

  public void setSmeN(int smeN) {
    sme.setSmeN(smeN);
  }

  public boolean isWantAlias() {
    return sme.getWantAlias();
  }

  public void setWantAlias(boolean wantAlias) {
    sme.setWantAlias(wantAlias);
  }

  public int getTimeout() {
    return sme.getTimeout();
  }

  public void setTimeout(int timeout) {
    sme.setTimeout(timeout);
  }

  public String getReceiptScheme() {
    return sme.getReceiptScheme();
  }

  public void setReceiptScheme(String receiptScheme) {
    sme.setReceiptScheme(receiptScheme);
  }

  public boolean isDisabled() {
    return sme.getDisabled();
  }

  public void setDisabled(boolean disabled) {
    sme.setDisabled(disabled);
  }

  public int getMode() {
    return sme.getMode().getValue();
  }

  public void setMode(int mode) {
    try {
      sme.setMode(SmeBindMode.valueOf((byte)mode));
    } catch (InvalidEnumValueException e) {
      sme.setMode(SmeBindMode.modeTrx);
    }
  }

  public int getProcLimit() {
    return sme.getProcLimit();
  }

  public void setProcLimit(int procLimit) {
    sme.setProcLimit(procLimit);
  }

  public int getSchedLimit() {
    return sme.getSchedLimit();
  }

  public void setSchedLimit(int schedLimit) {
    sme.setSchedLimit(schedLimit);
  }

  public int getAccessMask() {
    return sme.getAccessMask();
  }

  public void setAccessMask(int accessMask) {
    sme.setAccessMask(accessMask);
  }

  public boolean isFlagCarryOrgDescriptor() {
    return sme.getFlagCarryOrgDescriptor();
  }

  public void setFlagCarryOrgDescriptor(boolean flagCarryOrgDescriptor) {
    sme.setFlagCarryOrgDescriptor(flagCarryOrgDescriptor);
  }

  public boolean isFlagCarryOrgAbonentInfo() {
    return sme.getFlagCarryOrgAbonentInfo();
  }

  public void setFlagCarryOrgAbonentInfo(boolean flagCarryOrgAbonentInfo) {
    sme.setFlagCarryOrgAbonentInfo(flagCarryOrgAbonentInfo);
  }

  public boolean isFlagCarrySccpInfo() {
    return sme.getFlagCarrySccpInfo();
  }

  public void setFlagCarrySccpInfo(boolean flagCarrySccpInfo) {
    sme.setFlagCarrySccpInfo(flagCarrySccpInfo);
  }

  public boolean isFlagFillExtraDescriptor() {
    return sme.getFlagFillExtraDescriptor();
  }

  public void setFlagFillExtraDescriptor(boolean flagFillExtraDescriptor) {
    sme.setFlagFillExtraDescriptor(flagFillExtraDescriptor);
  }

  public boolean isFlagForceReceiptToSme() {
    return sme.getFlagForceReceiptToSme();
  }

  public void setFlagForceReceiptToSme(boolean flagForceReceiptToSme) {
    sme.setFlagForceReceiptToSme(flagForceReceiptToSme);
  }

  public boolean isFlagForceGsmDatacoding() {
    return sme.getFlagForceGsmDatacoding();
  }

  public void setFlagForceGsmDatacoding(boolean flagForceGsmDatacoding) {
    sme.setFlagForceGsmDatacoding(flagForceGsmDatacoding);
  }

  public boolean isFlagSmppPlus() {
    return sme.getFlagSmppPlus();
  }

  public void setFlagSmppPlus(boolean flagSmppPlus) {
    sme.setFlagSmppPlus(flagSmppPlus);
  }

  SmeParams toSmeParams() {
    return sme;
  }
}
