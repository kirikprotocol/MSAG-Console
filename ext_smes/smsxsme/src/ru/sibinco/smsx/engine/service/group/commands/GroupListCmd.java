package ru.sibinco.smsx.engine.service.group.commands;

import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.group.datasource.DistrList;

import java.util.List;
import java.util.Collection;

/**
 * User: artem
 * Date: 25.07.2008
 */

public class GroupListCmd extends GroupEditCommand {
  public String toString() {
    return "GroupListCmd: owner=" + owner;
  }

  public interface Receiver {
    public Collection<DistrList> execute(GroupListCmd cmd) throws CommandExecutionException;
  }
}
