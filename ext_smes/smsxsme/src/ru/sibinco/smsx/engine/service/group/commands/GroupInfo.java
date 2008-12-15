package ru.sibinco.smsx.engine.service.group.commands;

import java.util.List;
import java.util.Collection;

/**
 * User: artem
 * Date: 25.07.2008
 */

public class GroupInfo {

  private final Collection<String> members;
  private final Collection<String> submitters;
  private int maxElements;

  public GroupInfo(Collection<String> members, Collection<String> submitters) {
    this.members = members;
    this.submitters = submitters;
  }

  public Collection<String> getMembers() {
    return members;
  }

  public Collection<String> getSubmitters() {
    return submitters;
  }

  public int getMaxElements() {
    return maxElements;
  }

  public void setMaxElements(int maxElements) {
    this.maxElements = maxElements;
  }
}
