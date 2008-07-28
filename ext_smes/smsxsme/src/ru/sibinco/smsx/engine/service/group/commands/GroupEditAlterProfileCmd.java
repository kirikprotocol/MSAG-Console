package ru.sibinco.smsx.engine.service.group.commands;

import ru.sibinco.smsx.engine.service.Command;
import ru.sibinco.smsx.engine.service.CommandExecutionException;

/**
 * User: artem
 * Date: 25.07.2008
 */

public class GroupEditAlterProfileCmd extends Command {

  private String address;
  private Boolean sendNotifications;
  private Boolean lockGroupEdit;

  public String getAddress() {
    return address;
  }

  public void setAddress(String address) {
    this.address = address;
  }

  public Boolean getSendNotifications() {
    return sendNotifications;
  }

  public void setSendNotifications(Boolean sendNotifications) {
    this.sendNotifications = sendNotifications;
  }

  public Boolean getLockGroupEdit() {
    return lockGroupEdit;
  }

  public void setLockGroupEdit(Boolean lockGroupEdit) {
    this.lockGroupEdit = lockGroupEdit;
  }

  public interface Receiver {
    public void execute(GroupEditAlterProfileCmd cmd) throws CommandExecutionException;
  }
}
