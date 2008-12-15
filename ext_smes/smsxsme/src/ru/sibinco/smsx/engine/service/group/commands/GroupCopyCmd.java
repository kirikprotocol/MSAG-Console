package ru.sibinco.smsx.engine.service.group.commands;

import ru.sibinco.smsx.engine.service.CommandExecutionException;

/**
 * User: artem
 * Date: 28.07.2008
 */

public class GroupCopyCmd extends GroupEditCommand {
  private String newGroupName;

  public String getNewGroupName() {
    return newGroupName;
  }

  public void setNewGroupName(String newGroupName) {
    this.newGroupName = newGroupName;
  }

  public String toString() {
    return "GroupCopyCmd: name=" + groupName + "; owner=" + owner + "; newName=" + newGroupName;
  }

  public interface Receiver {
    public void execute(GroupCopyCmd cmd) throws CommandExecutionException;
  }
}
