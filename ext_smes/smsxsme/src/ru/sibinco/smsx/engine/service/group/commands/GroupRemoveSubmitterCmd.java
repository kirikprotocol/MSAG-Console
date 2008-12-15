package ru.sibinco.smsx.engine.service.group.commands;

import ru.sibinco.smsx.engine.service.CommandExecutionException;

/**
 * User: artem
 * Date: 21.11.2008
 */
public class GroupRemoveSubmitterCmd extends GroupEditCommand {
  private String submitter;

  public String getSubmitter() {
    return submitter;
  }

  public void setSubmitter(String submitter) {
    this.submitter = submitter;
  }

  public String toString() {
    return "GroupRemoveSubmitterCmd: name=" + groupName + "; owner=" + owner + "; submitter=" + submitter;
  }

  public interface Receiver {
    public void execute(GroupRemoveSubmitterCmd cmd) throws CommandExecutionException;
  }
}
