package ru.sibinco.smsx.engine.service.group.commands;

import ru.sibinco.smsx.engine.service.CommandExecutionException;

/**
 * User: artem
 * Date: 23.07.2008
 */

public class GroupAddCmd extends GroupEditCommand {

  public interface Receiver {
    public void execute(GroupAddCmd cmd) throws CommandExecutionException;
  }
}
