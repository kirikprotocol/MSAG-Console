package ru.sibinco.smsx.engine.service.group.commands;

import ru.sibinco.smsx.engine.service.CommandExecutionException;

/**
 * User: artem
 * Date: 23.07.2008
 */

public class GroupRemoveCmd extends GroupEditCommand {

  public interface Receiver {
    public void execute(GroupRemoveCmd cmd) throws CommandExecutionException;
  }

  public String toString() {
    return "GroupRemoveCmd: name=" + groupName + "; owner=" + owner;
  }
}
