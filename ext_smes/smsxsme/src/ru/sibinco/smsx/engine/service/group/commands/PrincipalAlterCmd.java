package ru.sibinco.smsx.engine.service.group.commands;

import ru.sibinco.smsx.engine.service.Command;
import ru.sibinco.smsx.engine.service.CommandExecutionException;

/**
 * User: artem
 * Date: 24.11.2008
 */
public class PrincipalAlterCmd extends Command {

  private String address;
  private int maxLists;
  private int maxElements;
  private boolean altLists;
  private boolean altElements;

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

  public boolean isAltLists() {
    return altLists;
  }

  public void setAltLists(boolean altLists) {
    this.altLists = altLists;
  }

  public boolean isAltElements() {
    return altElements;
  }

  public void setAltElements(boolean altElements) {
    this.altElements = altElements;
  }

  public String toString() {
    return "PrincipalAlterCmd: addr=" + address + "; maxLists=" + maxLists + "; maxElements=" + maxElements + "; altLists=" + altLists + "; altElements=" + altElements;
  }

  public interface Receiver {
    public void execute(PrincipalAlterCmd cmd) throws CommandExecutionException;
  }
}
