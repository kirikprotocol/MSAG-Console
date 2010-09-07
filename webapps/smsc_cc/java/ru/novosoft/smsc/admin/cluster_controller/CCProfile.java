package ru.novosoft.smsc.admin.cluster_controller;

import ru.novosoft.smsc.admin.cluster_controller.protocol.Profile;

/**
 * @author Artem Snopkov
 */
public class CCProfile {

  private String divert;
  private String locale;
  private boolean latin1;
  private boolean ucs2;
  private boolean ussdIn7Bit;
  private ReportOptions reportOptions;
  private HideOptions hide;
  private boolean hideModifiable;
  private boolean divertActive;
  private boolean divertActiveAbsent;
  private boolean divertActiveBlocked;
  private boolean divertActiveBarred;
  private boolean divertActiveCapacity;
  private boolean divertModifiable;
  private boolean udhConcat;
  private boolean translit;
  private Integer closedGroupId;
  private int accessMaskIn;
  private int accessMaskOut;
  private Integer subscription;
  private Byte sponsored;
  private String nick;

  public CCProfile() {

  }

  CCProfile(Profile p) {
    divert = p.getDivert();
    locale = p.getLocale();
    latin1 = (p.getCodepage() & 1) == 1;
    ucs2 = (p.getCodepage() & 8) == 8;
    ussdIn7Bit = (p.getCodepage() & 128) == 128;
    switch (p.getReportOptions()) {
      case ReportFinal: reportOptions=ReportOptions.ReportFinal; break;
      case ReportFull: reportOptions=ReportOptions.ReportFull; break;
      default:
        reportOptions = ReportOptions.ReportNone;
    }
    switch (p.getHide()) {
      case HideDisabled: hide = HideOptions.HideDisabled; break;
      case HideEnabled: hide = HideOptions.HideEnabled; break;
      default:
        hide = HideOptions.HideSubstitute;
    }
    hideModifiable = p.getHideModifiable();
    divertActive = p.getDivertActive();
    divertActiveAbsent = p.getDivertActiveAbsent();
    divertActiveBarred = p.getDivertActiveBarred();
    divertActiveBlocked = p.getDivertActiveBlocked();
    divertActiveCapacity = p.getDivertActiveCapacity();
    divertModifiable = p.getDivertModifiable();
    udhConcat = p.getUdhConcat();
    translit = p.getTranslit();
    if (p.getClosedGroupId() > 0)
      closedGroupId = p.getClosedGroupId();
    accessMaskIn = p.getAccessMaskIn();
    accessMaskOut = p.getAccessMaskOut();
    if (p.hasSubscription())
      subscription = p.getSubscription();
    if (p.hasSponsored())
      sponsored = p.getSponsored();
    if (p.hasNick())
      nick = p.getNick();
  }

  Profile toProfile() {
    Profile p = new Profile();
    p.setDivert(divert);
    p.setLocale(locale);
    int codepage = 0;
    if (latin1)
      codepage = codepage & 1;
    if (ucs2)
      codepage = codepage & 8;
    if (ussdIn7Bit)
      codepage = codepage & 128;
    p.setCodepage((byte)codepage);
    switch (reportOptions) {
      case ReportFinal: p.setReportOptions(ru.novosoft.smsc.admin.cluster_controller.protocol.ReportOptions.ReportFinal); break;
      case ReportFull: p.setReportOptions(ru.novosoft.smsc.admin.cluster_controller.protocol.ReportOptions.ReportFull); break;
      default:
        p.setReportOptions(ru.novosoft.smsc.admin.cluster_controller.protocol.ReportOptions.ReportNone);
    }
    switch (hide) {
      case HideDisabled: p.setHide(ru.novosoft.smsc.admin.cluster_controller.protocol.HideOptions.HideDisabled); break;
      case HideEnabled: p.setHide(ru.novosoft.smsc.admin.cluster_controller.protocol.HideOptions.HideEnabled); break;
      default:
        p.setHide(ru.novosoft.smsc.admin.cluster_controller.protocol.HideOptions.HideEnabled);
    }
    p.setHideModifiable(hideModifiable);
    p.setDivertActive(divertActive);
    p.setDivertActiveAbsent(divertActiveAbsent);
    p.setDivertActiveBarred(divertActiveBarred);
    p.setDivertActiveBlocked(divertActiveBlocked);
    p.setDivertActiveCapacity(divertActiveCapacity);
    p.setDivertModifiable(divertModifiable);
    p.setUdhConcat(udhConcat);
    p.setTranslit(translit);
    p.setClosedGroupId(closedGroupId == null ? 0 : closedGroupId);
    p.setAccessMaskIn(accessMaskIn);
    p.setAccessMaskOut(accessMaskOut);
    if (subscription != null)
      p.setSubscription(subscription);
    if (sponsored != null)
      p.setSponsored(sponsored);
    if (nick != null)
      p.setNick(nick);
    return p;


  }

  public String getDivert() {
    return divert;
  }

  public void setDivert(String divert) {
    this.divert = divert;
  }

  public String getLocale() {
    return locale;
  }

  public void setLocale(String locale) {
    this.locale = locale;
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

  public boolean isUssdIn7Bit() {
    return ussdIn7Bit;
  }

  public void setUssdIn7Bit(boolean ussdIn7Bit) {
    this.ussdIn7Bit = ussdIn7Bit;
  }

  public ReportOptions getReportOptions() {
    return reportOptions;
  }

  public void setReportOptions(ReportOptions reportOptions) {
    this.reportOptions = reportOptions;
  }

  public HideOptions getHide() {
    return hide;
  }

  public void setHide(HideOptions hide) {
    this.hide = hide;
  }

  public boolean isHideModifiable() {
    return hideModifiable;
  }

  public void setHideModifiable(boolean hideModifiable) {
    this.hideModifiable = hideModifiable;
  }

  public boolean isDivertActive() {
    return divertActive;
  }

  public void setDivertActive(boolean divertActive) {
    this.divertActive = divertActive;
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

  public Integer getClosedGroupId() {
    return closedGroupId;
  }

  public void setClosedGroupId(Integer closedGroupId) {
    this.closedGroupId = closedGroupId;
  }

  public int getAccessMaskIn() {
    return accessMaskIn;
  }

  public void setAccessMaskIn(int accessMaskIn) {
    this.accessMaskIn = accessMaskIn;
  }

  public int getAccessMaskOut() {
    return accessMaskOut;
  }

  public void setAccessMaskOut(int accessMaskOut) {
    this.accessMaskOut = accessMaskOut;
  }

  public int getSubscription() {
    return subscription;
  }

  public void setSubscription(Integer subscription) {
    this.subscription = subscription;
  }

  public byte getSponsored() {
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

  public enum HideOptions {
    HideDisabled, HideEnabled, HideSubstitute
  }

  public enum ReportOptions {
    ReportNone, ReportFull, ReportFinal
  }
}
