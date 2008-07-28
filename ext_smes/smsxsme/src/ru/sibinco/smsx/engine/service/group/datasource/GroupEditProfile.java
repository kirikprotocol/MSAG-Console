package ru.sibinco.smsx.engine.service.group.datasource;

/**
 * User: artem
 * Date: 25.07.2008
 */

public class GroupEditProfile {
  private final String address;
  private Boolean sendSmsNotification;
  private Boolean lockGroupEdit;

  public GroupEditProfile(String address) {
    this.address = address;
  }

  public Boolean getSendSmsNotification() {
    return sendSmsNotification;
  }

  public void setSendSmsNotification(Boolean sendSmsNotification) {
    this.sendSmsNotification = sendSmsNotification;
  }

  public boolean isSendSmsNotification() {
    return sendSmsNotification;
  }

  public Boolean getLockGroupEdit() {
    return lockGroupEdit;
  }

  public boolean isLockGroupEdit() {
    return lockGroupEdit;
  }

  public void setLockGroupEdit(Boolean lockGroupEdit) {
    this.lockGroupEdit = lockGroupEdit;
  }

  public String getAddress() {
    return address;
  }
}
