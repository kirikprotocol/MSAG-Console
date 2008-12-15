package ru.sibinco.smsx.engine.service.group.commands;

import ru.sibinco.smsx.engine.service.CommandExecutionException;

/**
 * User: artem
 * Date: 02.12.2008
 */
public class GroupReplyCmd extends GroupSendCommand {

  public interface Receiver {
    public long execute(GroupReplyCmd cmd) throws CommandExecutionException;
  }
}
