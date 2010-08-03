package ru.novosoft.smsc.admin.sme;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.util.ValidationHelper;

/**
 * Структура для хранения настроек Sme
 *
 * @author Artem Snopkov
 */
public class Sme {

  private static final ValidationHelper vh = new ValidationHelper(Sme.class);

  private int priority = 0;
  private String systemType = null;
  private String password = null;
  private String addrRange = null;
  private int smeN = 0;
  private boolean wantAlias = false;
  private int timeout = 0;

  private String receiptSchemeName = null;
  private boolean disabled = false;
  private SmeBindMode bindMode = SmeBindMode.TRX;
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


  public Sme() {
  }

  Sme(Sme newSme) {
    priority = newSme.getPriority();

    systemType = newSme.getSystemType();
    password = newSme.getPassword();
    addrRange = newSme.getAddrRange();
    smeN = newSme.getSmeN();
    wantAlias = newSme.isWantAlias();
    timeout = newSme.getTimeout();

    receiptSchemeName = newSme.getReceiptSchemeName();
    disabled = newSme.isDisabled();
    bindMode = newSme.getBindMode();
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
  }

  public int getPriority() {
    return priority;
  }

  public void setPriority(int priority) throws AdminException {
    vh.checkGreaterOrEqualsTo("priority", priority, 0);
    this.priority = priority;
  }

  public String getSystemType() {
    return systemType;
  }

  public void setSystemType(String systemType) throws AdminException {
    vh.checkNotNull("systemType", systemType);
    this.systemType = systemType;
  }

  public String getPassword() {
    return password;
  }

  public void setPassword(String password) throws AdminException {
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

  public void setTimeout(int timeout) throws AdminException {
    vh.checkPositive("timeout", timeout);
    this.timeout = timeout;
  }

  public String getReceiptSchemeName() {
    return receiptSchemeName;
  }

  // todo здесь должен кидаться Exception, если не существует ресурса с именем, указанным в receiptSchemeName
  public void setReceiptSchemeName(String receiptSchemeName) throws AdminException {
    vh.checkNotNull("receiptSchemeName", receiptSchemeName); // TODO DEPENDENCY !!!
    this.receiptSchemeName = receiptSchemeName;
  }

  public boolean isDisabled() {
    return disabled;
  }

  public void setDisabled(boolean disabled) {
    this.disabled = disabled;
  }

  public SmeBindMode getBindMode() {
    return bindMode;
  }

  public void setBindMode(SmeBindMode bindMode) throws AdminException {
    vh.checkNotNull("bindMode", bindMode);
    this.bindMode = bindMode;
  }

  public int getProclimit() {
    return proclimit;
  }

  public void setProclimit(int proclimit) throws AdminException {
    vh.checkGreaterOrEqualsTo("proclimit", proclimit, 0);
    this.proclimit = proclimit;
  }

  public int getSchedlimit() {
    return schedlimit;
  }

  public void setSchedlimit(int schedlimit) throws AdminException {
    vh.checkGreaterOrEqualsTo("schedlimit", schedlimit, 0);
    this.schedlimit = schedlimit;
  }

  public int getAccessMask() {
    return accessMask;
  }

  public void setAccessMask(int accessMask) throws AdminException {
    vh.checkGreaterOrEqualsTo("accessMask", accessMask, 0);
    this.accessMask = accessMask;
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
}
