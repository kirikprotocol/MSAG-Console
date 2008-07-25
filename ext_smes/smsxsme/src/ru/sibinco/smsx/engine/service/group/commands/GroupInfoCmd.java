package ru.sibinco.smsx.engine.service.group.commands;

import ru.sibinco.smsx.engine.service.CommandExecutionException;

/**
 * User: artem
 * Date: 25.07.2008
 */

public class GroupInfoCmd extends GroupCommand {
  public interface Receiver {
    public GroupInfo execute(GroupInfoCmd cmd) throws CommandExecutionException;
  }
}
