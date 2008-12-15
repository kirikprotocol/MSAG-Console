package ru.sibinco.smsx.engine.service.group.commands;

import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.group.DeliveryStatus;

/**
 * User: artem
 * Date: 15.07.2008
 */

public class GroupSendCmd extends GroupSendCommand {

  // Input parameters
  private String groupName;
  private String owner;

  public String getGroupName() {
    return groupName;
  }

  public GroupSendCmd setGroupName(String groupName) {
    this.groupName = groupName;
    return this;
  }

  public String getOwner() {
    return owner;
  }

  public GroupSendCmd setOwner(String owner) {
    this.owner = owner;
    return this;
  }

  public interface Receiver {
    public long execute(GroupSendCmd cmd) throws CommandExecutionException;
  }
}
