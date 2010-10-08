package ru.novosoft.smsc.admin.profile;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.util.ValidationHelper;
import ru.novosoft.smsc.util.Address;

import java.io.Serializable;

/**
 * Структура, описывающая профиль
 * @author Artem Snopkov
 */
public class Profile implements Serializable {

  private static final transient ValidationHelper vh = new ValidationHelper(Profile.class);

  private Address address;

  // unconditional, absent, blocked, barred, capacity
  private String divert;
  private boolean divertActive;
  private boolean divertActiveAbsent = false;
  private boolean divertActiveBlocked = false;
  private boolean divertActiveBarred = false;
  private boolean divertActiveCapacity = false;
  private boolean divertModifiable = false;

  private boolean latin1;
  private boolean ucs2;
  private boolean ussd7bit;

  private ReportOption reportOptions = ReportOption.NONE;
  private String locale;

  private AliasHide aliasHide = AliasHide.TRUE;
  private boolean aliasModifiable = false;
  private boolean udhConcat = true;
  private boolean translit = true;

  private Integer groupId;
  private int inputAccessMask = 1;
  private int outputAccessMask = 1;

  private Byte sponsored = 0;
  private String nick = null;
  private Integer subscription;

  public Profile(Address address) throws AdminException {
    vh.checkNotNull("address", address);
    this.address = address;
  }

  public Profile() {    
  }

  protected Profile(Profile copy) {
    address = copy.address;
    
    divert = copy.divert;
    divertActive = copy.divertActive;
    divertActiveAbsent = copy.divertActiveAbsent;
    divertActiveBlocked = copy.divertActiveBlocked;
    divertActiveBarred = copy.divertActiveBarred;
    divertActiveCapacity = copy.divertActiveCapacity;
    divertModifiable = copy.divertModifiable;

    latin1 = copy.latin1;
    ucs2 = copy.ucs2;
    ussd7bit = copy.ussd7bit;
    reportOptions = copy.reportOptions;
    locale = copy.locale;
    aliasHide = copy.aliasHide;
    aliasModifiable = copy.aliasModifiable;
    udhConcat = copy.udhConcat;
    translit = copy.translit;
    groupId = copy.groupId;
    inputAccessMask = copy.inputAccessMask;
    outputAccessMask = copy.outputAccessMask;
    sponsored = copy.sponsored;
    nick = copy.nick;
    subscription = copy.subscription;
  }



  public Address getAddress() {
    return address;
  }

  public void setAddress(Address address) throws AdminException {
    vh.checkNotNull("address", address);
    this.address = address;
  }

  public boolean isDivertActive() {
    return divertActive;
  }

  public void setDivertActive(boolean divertActive) {
    this.divertActive = divertActive;
  }

  public boolean isUssd7bit() {
    return ussd7bit;
  }

  public void setUssd7bit(boolean ussd7bit) {
    this.ussd7bit = ussd7bit;
  }

  public String getDivert() {
    return divert;
  }

  public void setDivert(String divert) {
    this.divert = divert;
  }  

  public boolean isDivertActiveAbsent() {
    return divertActiveAbsent;
  }

  public void setDivertActiveAbsent(boolean divertActiveAbsent) {
    this.divertActiveAbsent = divertActiveAbsent;
  }

  public boolean isDivertActiveBlocked() {
    return divertActiveBlocked;
  }

  public void setDivertActiveBlocked(boolean divertActiveBlocked) {
    this.divertActiveBlocked = divertActiveBlocked;
  }

  public boolean isDivertActiveBarred() {
    return divertActiveBarred;
  }

  public void setDivertActiveBarred(boolean divertActiveBarred) {
    this.divertActiveBarred = divertActiveBarred;
  }

  public boolean isDivertActiveCapacity() {
    return divertActiveCapacity;
  }

  public void setDivertActiveCapacity(boolean divertActiveCapacity) {
    this.divertActiveCapacity = divertActiveCapacity;
  }

  public boolean isDivertModifiable() {
    return divertModifiable;
  }

  public void setDivertModifiable(boolean divertModifiable) {
    this.divertModifiable = divertModifiable;
  }

  public boolean isLatin1() {
    return latin1;
  }

  public void setLatin1(boolean latin1) {
    this.latin1 = latin1;
  }

  public boolean isUcs2() {
    return ucs2;
  }

  public void setUcs2(boolean ucs2) {
    this.ucs2 = ucs2;
  }

  public ReportOption getReportOptions() {
    return reportOptions;
  }

  public void setReportOptions(ReportOption reportOptions) throws AdminException {
    vh.checkNotNull("reportOptions", reportOptions);
    this.reportOptions = reportOptions;
  }

  public String getLocale() {
    return locale;
  }

  public void setLocale(String locale) throws AdminException {
    vh.checkNotNull("locale", locale);
    this.locale = locale;
  }

  public AliasHide getAliasHide() {
    return aliasHide;
  }

  public void setAliasHide(AliasHide aliasHide) throws AdminException {
    vh.checkNotNull("aliasHide", aliasHide);
    this.aliasHide = aliasHide;
  }

  public boolean isAliasModifiable() {
    return aliasModifiable;
  }

  public void setAliasModifiable(boolean aliasModifiable) {
    this.aliasModifiable = aliasModifiable;
  }

  public boolean isUdhConcat() {
    return udhConcat;
  }

  public void setUdhConcat(boolean udhConcat) {
    this.udhConcat = udhConcat;
  }

  public boolean isTranslit() {
    return translit;
  }

  public void setTranslit(boolean translit) {
    this.translit = translit;
  }

  public Integer getGroupId() {
    return groupId;
  }

  public void setGroupId(Integer groupId) {
    this.groupId = groupId;
  }

  public int getInputAccessMask() {
    return inputAccessMask;
  }

  public void setInputAccessMask(int inputAccessMask) {
    this.inputAccessMask = inputAccessMask;
  }

  public int getOutputAccessMask() {
    return outputAccessMask;
  }

  public void setOutputAccessMask(int outputAccessMask) {
    this.outputAccessMask = outputAccessMask;
  }

  public Byte getSponsored() {
    return sponsored;
  }

  public void setSponsored(Byte sponsored) {
    this.sponsored = sponsored;
  }

  public String getNick() {
    return nick;
  }

  public void setNick(String nick) {
    this.nick = nick;
  }

  public Integer getSubscription() {
    return subscription;
  }

  public void setSubscription(Integer subscription) {
    this.subscription = subscription;
  }

  @Override
  public String toString() {
    return "Profile{" +
        "address=" + address +
        ", divert='" + divert + '\'' +
        ", divertActive=" + divertActive +
        ", divertActiveAbsent=" + divertActiveAbsent +
        ", divertActiveBlocked=" + divertActiveBlocked +
        ", divertActiveBarred=" + divertActiveBarred +
        ", divertActiveCapacity=" + divertActiveCapacity +
        ", divertModifiable=" + divertModifiable +
        ", latin1=" + latin1 +
        ", ucs2=" + ucs2 +
        ", ussd7bit=" + ussd7bit +
        ", reportOptions=" + reportOptions +
        ", locale='" + locale + '\'' +
        ", aliasHide=" + aliasHide +
        ", aliasModifiable=" + aliasModifiable +
        ", udhConcat=" + udhConcat +
        ", translit=" + translit +
        ", groupId=" + groupId +
        ", inputAccessMask=" + inputAccessMask +
        ", outputAccessMask=" + outputAccessMask +
        ", sponsored=" + sponsored +
        ", nick='" + nick + '\'' +
        ", subscription=" + subscription +
        '}';
  }
}
