package ru.sibinco.smsx.engine.service.group.commands;

import ru.sibinco.smsx.engine.service.Command;
import ru.sibinco.smsx.engine.service.CommandExecutionException;

/**
 * User: artem
 * Date: 21.11.2008
 */
public class PrincipalAddCmd extends GroupEditCommand {

  private String address;
  private int maxLists;
  private int maxElements;

  public String getAddress() {
    return address;
  }

  public void setAddress(String address) {
    this.address = address;
  }

  public int getMaxLists() {
    return maxLists;
  }

  public void setMaxLists(int maxLists) {
    this.maxLists = maxLists;
  }

  public int getMaxElements() {
    return maxElements;
  }

  public void setMaxElements(int maxElements) {
    this.maxElements = maxElements;
  }

  public String toString() {
    return "PrincipalAddCmd: addr=" + address + "; maxLists=" + maxLists + "; maxElements=" + maxElements;
  }

  public interface Receiver {
    public void execute(PrincipalAddCmd cmd) throws CommandExecutionException;
  }
}
