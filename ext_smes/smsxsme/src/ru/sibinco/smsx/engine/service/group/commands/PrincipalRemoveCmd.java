package ru.sibinco.smsx.engine.service.group.commands;

import ru.sibinco.smsx.engine.service.CommandExecutionException;

/**
 * User: artem
 * Date: 21.11.2008
 */
public class PrincipalRemoveCmd extends GroupEditCommand {  

  private String address;

  public String getAddress() {
    return address;
  }

  public void setAddress(String address) {
    this.address = address;
  }

  public String toString() {
    return "PrincipalRemoveCmd: addr=" + address;
  }

  public interface Receiver {
    public void execute(PrincipalRemoveCmd cmd) throws CommandExecutionException;
  }
}
