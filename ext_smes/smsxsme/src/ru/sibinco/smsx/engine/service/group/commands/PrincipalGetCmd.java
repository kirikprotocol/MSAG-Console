package ru.sibinco.smsx.engine.service.group.commands;

import ru.sibinco.smsx.engine.service.Command;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.group.datasource.Principal;

/**
 * User: artem
 * Date: 24.11.2008
 */
public class PrincipalGetCmd extends Command {
  private String address;

  public String getAddress() {
    return address;
  }

  public void setAddress(String address) {
    this.address = address;
  }

  public String toString() {
    return "PrincipalGetCmd: addr=" + address ;
  }

  public interface Receiver {
    public Principal execute(PrincipalGetCmd cmd) throws CommandExecutionException;
  }
}
