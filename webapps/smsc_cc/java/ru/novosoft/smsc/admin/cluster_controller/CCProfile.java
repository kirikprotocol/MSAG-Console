package ru.novosoft.smsc.admin.cluster_controller;

import ru.novosoft.smsc.admin.cluster_controller.protocol.Profile;

/**
 * @author Artem Snopkov
 */
public class CCProfile {

  private final Profile profile ;

  public CCProfile() {
    this.profile = new Profile();
    profile.setAccessMaskIn(0);
    profile.setAccessMaskOut(0);
    profile.setDivertActive(false);
    profile.setDivertActiveAbsent(false);
    profile.setDivertActiveBarred(false);
    profile.setDivertActiveCapacity(false);
    profile.setDivertModifiable(false);
    profile.setDivert("");
    profile.setDivertActiveBlocked(false);
    profile.setCodepage((byte)0);
    profile.setHide(ru.novosoft.smsc.admin.cluster_controller.protocol.HideOptions.HideDisabled);
    profile.setHideModifiable(false);
    profile.setLocale("");
    profile.setReportOptions(ru.novosoft.smsc.admin.cluster_controller.protocol.ReportOptions.ReportNone);
    profile.setUdhConcat(false);
    profile.setTranslit(false);
    
  }

  CCProfile(Profile p) {
    this.profile = p;
  }

  Profile toProfile() {
    return profile;
  }

  public String getDivert() {
    return profile.getDivert();
  }

  public void setDivert(String divert) {
    profile.setDivert(divert);
  }

  public String getLocale() {
    return profile.getLocale();
  }

  public void setLocale(String locale) {
    profile.setLocale(locale);
  }

  public boolean isLatin1() {
    return (profile.getCodepage() & 1) == 1;
  }

  public void setLatin1(boolean latin1) {
    profile.setCodepage((byte)(profile.getCodepage() | 1));
  }

  public boolean isUcs2() {
    return (profile.getCodepage() & 8) == 8;
  }

  public void setUcs2(boolean ucs2) {
    profile.setCodepage((byte)(profile.getCodepage() | 8));
  }

  public boolean isUssdIn7Bit() {
    return (profile.getCodepage() & 128) == 128;
  }

  public void setUssdIn7Bit(boolean ussdIn7Bit) {
    profile.setCodepage((byte)(profile.getCodepage() | 128));
  }

  public ReportOptions getReportOptions() {
    switch (profile.getReportOptions()) {
      case ReportFinal: return ReportOptions.ReportFinal;
      case ReportFull: return ReportOptions.ReportFull;
      default:
        return ReportOptions.ReportNone;
    }
  }

  public void setReportOptions(ReportOptions reportOptions) {
    switch (reportOptions) {
      case ReportFinal: profile.setReportOptions(ru.novosoft.smsc.admin.cluster_controller.protocol.ReportOptions.ReportFinal); break;
      case ReportFull: profile.setReportOptions(ru.novosoft.smsc.admin.cluster_controller.protocol.ReportOptions.ReportFull); break;
      default:
        profile.setReportOptions(ru.novosoft.smsc.admin.cluster_controller.protocol.ReportOptions.ReportNone);
    }
  }

  public HideOptions getHide() {
     switch (profile.getHide()) {
      case HideDisabled: return HideOptions.HideDisabled;
      case HideEnabled: return HideOptions.HideEnabled;
      default:
        return HideOptions.HideSubstitute;
    }
  }

  public void setHide(HideOptions hide) {
    switch (hide) {
      case HideDisabled: profile.setHide(ru.novosoft.smsc.admin.cluster_controller.protocol.HideOptions.HideDisabled); break;
      case HideEnabled: profile.setHide(ru.novosoft.smsc.admin.cluster_controller.protocol.HideOptions.HideEnabled); break;
      default:
        profile.setHide(ru.novosoft.smsc.admin.cluster_controller.protocol.HideOptions.HideEnabled);
    }
  }

  public boolean isHideModifiable() {
    return profile.getHideModifiable();
  }

  public void setHideModifiable(boolean hideModifiable) {
    profile.setHideModifiable(hideModifiable);
  }

  public boolean isDivertActive() {
    return profile.getDivertActive();
  }

  public void setDivertActive(boolean divertActive) {
    profile.setDivertActive(divertActive);
  }

  public boolean isDivertActiveAbsent() {
    return profile.getDivertActiveAbsent();
  }

  public void setDivertActiveAbsent(boolean divertActiveAbsent) {
    profile.setDivertActiveAbsent(divertActiveAbsent);
  }

  public boolean isDivertActiveBlocked() {
    return profile.getDivertActiveBlocked();
  }

  public void setDivertActiveBlocked(boolean divertActiveBlocked) {
    profile.setDivertActiveBlocked(divertActiveBlocked);
  }

  public boolean isDivertActiveBarred() {
    return profile.getDivertActiveBarred();
  }

  public void setDivertActiveBarred(boolean divertActiveBarred) {
    profile.setDivertActiveBarred(divertActiveBarred);
  }

  public boolean isDivertActiveCapacity() {
    return profile.getDivertActiveCapacity();
  }

  public void setDivertActiveCapacity(boolean divertActiveCapacity) {
    profile.setDivertActiveCapacity(divertActiveCapacity);
  }

  public boolean isDivertModifiable() {
    return profile.getDivertModifiable();
  }

  public void setDivertModifiable(boolean divertModifiable) {
    profile.setDivertModifiable(divertModifiable);
  }

  public boolean isUdhConcat() {
    return profile.getUdhConcat();
  }

  public void setUdhConcat(boolean udhConcat) {
    profile.setUdhConcat(udhConcat);
  }

  public boolean isTranslit() {
    return profile.getTranslit();
  }

  public void setTranslit(boolean translit) {
    profile.setTranslit(translit);
  }

  public Integer getClosedGroupId() {
    return !profile.hasClosedGroupId() || profile.getClosedGroupId() == 0 ? null : profile.getClosedGroupId();
  }

  public void setClosedGroupId(Integer closedGroupId) {
    profile.setClosedGroupId(closedGroupId == null ? 0 : closedGroupId);
  }

  public int getAccessMaskIn() {
    return profile.getAccessMaskIn();
  }

  public void setAccessMaskIn(int accessMaskIn) {
    profile.setAccessMaskIn(accessMaskIn);
  }

  public int getAccessMaskOut() {
    return profile.getAccessMaskOut();
  }

  public void setAccessMaskOut(int accessMaskOut) {
    profile.setAccessMaskOut(accessMaskOut);
  }

  public Integer getSubscription() {
    if (profile.hasSubscription())
      return profile.getSubscription();
    else
      return null;
  }

  public void setSubscription(Integer subscription) {
    if (subscription != null)
      profile.setSubscription(subscription);
  }

  public Byte getSponsored() {
    if (profile.hasSponsored())
      return profile.getSponsored();
    else
      return null;
  }

  public void setSponsored(byte sponsored) {
    profile.setSponsored(sponsored);
  }

  public String getNick() {
    if (profile.hasNick())
      return profile.getNick();
    else
      return null;
  }

  public void setNick(String nick) {
    profile.setNick(nick);
  }

  public enum HideOptions {
    HideDisabled, HideEnabled, HideSubstitute
  }

  public enum ReportOptions {
    ReportNone, ReportFull, ReportFinal
  }
}
