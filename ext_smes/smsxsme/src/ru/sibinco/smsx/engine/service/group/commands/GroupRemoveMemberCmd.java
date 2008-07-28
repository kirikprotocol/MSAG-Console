package ru.sibinco.smsx.engine.service.group.commands;

import ru.sibinco.smsx.engine.service.CommandExecutionException;

/**
 * User: artem
 * Date: 23.07.2008
 */

public class GroupRemoveMemberCmd extends GroupEditCommand {
  private String member;

  public String getMember() {
    return member;
  }

  public void setMember(String member) {
    this.member = member;
  }

  public interface Receiver {
    public void execute(GroupRemoveMemberCmd cmd) throws CommandExecutionException;
  }
}
