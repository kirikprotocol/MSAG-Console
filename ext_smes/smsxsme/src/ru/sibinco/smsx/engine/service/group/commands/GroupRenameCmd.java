package ru.sibinco.smsx.engine.service.group.commands;

import ru.sibinco.smsx.engine.service.CommandExecutionException;

/**
 * User: artem
 * Date: 23.07.2008
 */

public class GroupRenameCmd extends GroupEditCommand {

  private String newGroupName;

  public String getNewGroupName() {
    return newGroupName;
  }

  public void setNewGroupName(String newGroupName) {
    this.newGroupName = newGroupName;
  }

  public String toString() {
    return "GroupRenameCmd: name=" + groupName + "; owner=" + owner + "; newName=" + newGroupName;
  }

  public interface Receiver {
    public void execute(GroupRenameCmd cmd) throws CommandExecutionException;
  }
}
