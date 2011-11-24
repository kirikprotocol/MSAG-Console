package ru.novosoft.smsc.admin.mcisme;

import ru.novosoft.smsc.util.Address;

/**
 * author: Aleksandr Khalitov
 */
public class Profile {

  private Address subscriber;
  private boolean inform;
  private boolean notify;
  private boolean wantNotifyMe;
  private boolean busy;
  private boolean noReplay;
  private boolean absent;
  private boolean detach;
  private boolean unconditional;
  private byte informTemplateId;
  private byte notifyTemplateId;

  public Address getSubscriber() {
    return subscriber;
  }

  public void setSubscriber(Address subscriber) {
    this.subscriber = subscriber;
  }

  public boolean isBusy() {
    return busy;
  }

  public void setBusy(boolean busy) {
    this.busy = busy;
  }

  public boolean isNoReplay() {
    return noReplay;
  }

  public void setNoReplay(boolean noReplay) {
    this.noReplay = noReplay;
  }

  public boolean isAbsent() {
    return absent;
  }

  public void setAbsent(boolean absent) {
    this.absent = absent;
  }

  public boolean isDetach() {
    return detach;
  }

  public void setDetach(boolean detach) {
    this.detach = detach;
  }

  public boolean isUnconditional() {
    return unconditional;
  }

  public void setUnconditional(boolean unconditional) {
    this.unconditional = unconditional;
  }

  public boolean isInform() {
    return inform;
  }

  public void setInform(boolean inform) {
    this.inform = inform;
  }

  public boolean isNotify() {
    return notify;
  }

  public void setNotify(boolean notify) {
    this.notify = notify;
  }

  public boolean isWantNotifyMe() {
    return wantNotifyMe;
  }

  public void setWantNotifyMe(boolean wantNotifyMe) {
    this.wantNotifyMe = wantNotifyMe;
  }

  public byte getInformTemplateId() {
    return informTemplateId;
  }

  public void setInformTemplateId(byte informTemplateId) {
    this.informTemplateId = informTemplateId;
  }

  public byte getNotifyTemplateId() {
    return notifyTemplateId;
  }

  public void setNotifyTemplateId(byte notifyTemplateId) {
    this.notifyTemplateId = notifyTemplateId;
  }

  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    Profile profile = (Profile) o;

    if (absent != profile.absent) return false;
    if (busy != profile.busy) return false;
    if (detach != profile.detach) return false;
    if (inform != profile.inform) return false;
    if (informTemplateId != profile.informTemplateId) return false;
    if (noReplay != profile.noReplay) return false;
    if (notify != profile.notify) return false;
    if (notifyTemplateId != profile.notifyTemplateId) return false;
    if (unconditional != profile.unconditional) return false;
    if (wantNotifyMe != profile.wantNotifyMe) return false;
    if (subscriber != null ? !subscriber.equals(profile.subscriber) : profile.subscriber != null) return false;

    return true;
  }

  @SuppressWarnings({"ConstantConditions"})
  @Override
  public int hashCode() {
    int result = subscriber != null ? subscriber.hashCode() : 0;
    result = 31 * result + (inform ? 1 : 0);
    result = 31 * result + (notify ? 1 : 0);
    result = 31 * result + (wantNotifyMe ? 1 : 0);
    result = 31 * result + (busy ? 1 : 0);
    result = 31 * result + (noReplay ? 1 : 0);
    result = 31 * result + (absent ? 1 : 0);
    result = 31 * result + (detach ? 1 : 0);
    result = 31 * result + (unconditional ? 1 : 0);
    result = 31 * result + (int) informTemplateId;
    result = 31 * result + (int) notifyTemplateId;
    return result;
  }
}
