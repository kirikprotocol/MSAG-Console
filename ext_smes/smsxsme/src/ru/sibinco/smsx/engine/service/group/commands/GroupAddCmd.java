package ru.sibinco.smsx.engine.service.group.commands;

import ru.sibinco.smsx.engine.service.CommandExecutionException;

/**
 * User: artem
 * Date: 23.07.2008
 */

public class GroupAddCmd extends GroupEditCommand {

  private int maxElements = -1;

  public int getMaxElements() {
    return maxElements;
  }

  public void setMaxElements(int maxElements) {
    this.maxElements = maxElements;
  }

  public interface Receiver {
    public void execute(GroupAddCmd cmd) throws CommandExecutionException;
  }

  public String toString() {
    return "GroupAddCmd: name=" + groupName + "; owner=" + owner + "; max=" + maxElements;
  }
}
