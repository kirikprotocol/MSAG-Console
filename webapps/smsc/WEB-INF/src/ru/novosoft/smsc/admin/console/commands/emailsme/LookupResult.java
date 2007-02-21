package ru.novosoft.smsc.admin.console.commands.emailsme;

/**
 * User: artem
 * Date: 12.09.2006
 */

public class LookupResult {
  private byte ton;
  private byte npi;
  private String addr;
  private String userName;
  private String forwardEmail;
  private String realName;
  private byte limitType;
  private byte numberMapping;
  private int lastLimitUpdateDate;
  private int limitValue;
  private int limitCountGsm2Eml;
  private int limitCountEml2Gsm;

  public void setTon(byte ton) {
    this.ton = ton;
  }

  public void setNpi(byte npi) {
    this.npi = npi;
  }

  public void setAddr(String addr) {
    this.addr = addr;
  }

  public void setUserName(String userName) {
    this.userName = userName;
  }

  public void setForwardEmail(String forwardEmail) {
    this.forwardEmail = forwardEmail;
  }

  public void setRealName(String realName) {
    this.realName = realName;
  }

  public void setLimitType(byte limitType) {
    this.limitType = limitType;
  }

  public void setNumberMapping(byte numberMapping) {
    this.numberMapping = numberMapping;
  }

  public void setLastLimitUpdateDate(int lastLimitUpdateDate) {
    this.lastLimitUpdateDate = lastLimitUpdateDate;
  }

  public void setLimitValue(int limitValue) {
    this.limitValue = limitValue;
  }

  public void setLimitCountGsm2Eml(int limitCountGsm2Eml) {
    this.limitCountGsm2Eml = limitCountGsm2Eml;
  }

  public void setLimitCountEml2Gsm(int limitCountEml2Gsm) {
    this.limitCountEml2Gsm = limitCountEml2Gsm;
  }

  public byte getTon() {
    return ton;
  }

  public byte getNpi() {
    return npi;
  }

  public String getAddr() {
    return addr;
  }

  public String getUserName() {
    return userName;
  }

  public String getForwardEmail() {
    return forwardEmail;
  }

  public String getRealName() {
    return realName;
  }

  public byte getLimitType() {
    return limitType;
  }

  public byte getNumberMapping() {
    return numberMapping;
  }

  public int getLastLimitUpdateDate() {
    return lastLimitUpdateDate;
  }

  public int getLimitValue() {
    return limitValue;
  }

  public int getLimitCountGsm2Eml() {
    return limitCountGsm2Eml;
  }

  public int getLimitCountEml2Gsm() {
    return limitCountEml2Gsm;
  }

  public String toString() {
    final StringBuffer buffer = new StringBuffer();
    final char ch = 0x000D;
    final String nextStr = "" + '\n' + ch;
    buffer.append(" Ton = ").append(ton).append(nextStr);
    buffer.append(" Npi = ").append(npi).append(nextStr);
    buffer.append(" Addr = ").append(addr).append(nextStr);
    buffer.append(" User name = ").append(userName).append(nextStr);
    buffer.append(" Forward email = ").append(forwardEmail).append(nextStr);
    buffer.append(" Real name = ").append(realName).append(nextStr);
    buffer.append(" Limit type = ").append(limitType).append(nextStr);
    buffer.append(" Number mapping = ").append(numberMapping).append(nextStr);
    buffer.append(" Last limit update date = ").append(lastLimitUpdateDate).append(nextStr);
    buffer.append(" Limit value = ").append(limitValue).append(nextStr);
    buffer.append(" Limit count GSM to Email = ").append(limitCountGsm2Eml).append(nextStr);
    buffer.append(" Limit count email to GSM = ").append(limitCountEml2Gsm).append(nextStr);
    return buffer.toString();
  }
}