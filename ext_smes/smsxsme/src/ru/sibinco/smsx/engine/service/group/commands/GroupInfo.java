package ru.sibinco.smsx.engine.service.group.commands;

import java.util.List;

/**
 * User: artem
 * Date: 25.07.2008
 */

public class GroupInfo {
  private final List<String> members;

  public GroupInfo(List<String> members) {
    this.members = members;
  }

  public List<String> getMembers() {
    return members;
  }
}
