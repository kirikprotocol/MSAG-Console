package ru.novosoft.smsc.admin.mcisme;

import ru.novosoft.smsc.util.Address;

/**
 * author: Aleksandr Khalitov
 */
public class Profile {

  private Address subscriber;
  private byte eventMask;
  private boolean inform;
  private boolean notify;
  private boolean wantNotifyMe;
  private byte informTemplateId;
  private byte notifyTemplateId;

  public Address getSubscriber() {
    return subscriber;
  }

  public void setSubscriber(Address subscriber) {
    this.subscriber = subscriber;
  }

  public byte getEventMask() {
    return eventMask;
  }

  public void setEventMask(byte eventMask) {
    this.eventMask = eventMask;
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

  @SuppressWarnings({"OverlyComplexMethod"})
  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    Profile profile = (Profile) o;

    if (eventMask != profile.eventMask) return false;
    if (inform != profile.inform) return false;
    if (informTemplateId != profile.informTemplateId) return false;
    if (notify != profile.notify) return false;
    if (notifyTemplateId != profile.notifyTemplateId) return false;
    if (wantNotifyMe != profile.wantNotifyMe) return false;
    if (subscriber != null ? !subscriber.equals(profile.subscriber) : profile.subscriber != null) return false;

    return true;
  }

  @Override
  public int hashCode() {
    int result = subscriber != null ? subscriber.hashCode() : 0;
    result = 31 * result + (int) eventMask;
    result = 31 * result + (inform ? 1 : 0);
    result = 31 * result + (notify ? 1 : 0);
    result = 31 * result + (wantNotifyMe ? 1 : 0);
    result = 31 * result + (int) informTemplateId;
    result = 31 * result + (int) notifyTemplateId;
    return result;
  }
}
