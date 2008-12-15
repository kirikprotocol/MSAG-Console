package ru.sibinco.smsx.engine.service.group.commands;

import ru.sibinco.smsx.engine.service.CommandExecutionException;

/**
 * User: artem
 * Date: 25.11.2008
 */
public class GroupAlterCmd extends GroupEditCommand {
  private int maxElements;

  public int getMaxElements() {
    return maxElements;
  }

  public void setMaxElements(int maxElements) {
    this.maxElements = maxElements;
  }

  public String toString() {
    return "GroupAlterCmd: name=" + groupName + "; owner=" + owner + "; max=" + maxElements;
  }

  public interface Receiver {
    public void execute(GroupAlterCmd cmd) throws CommandExecutionException;
  }
}
