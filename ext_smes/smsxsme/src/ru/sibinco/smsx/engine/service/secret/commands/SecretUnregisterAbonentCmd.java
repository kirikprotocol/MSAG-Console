package ru.sibinco.smsx.engine.service.secret.commands;

import ru.sibinco.smsx.engine.service.Command;

/**
 * User: artem
 * Date: Sep 11, 2007
 */

public class SecretUnregisterAbonentCmd extends Command {

  public static final int STATUS_SOURCE_ABONENT_NOT_REGISTERED = STATUS_INT + 1;

  private String abonentAddress;

  public String getAbonentAddress() {
    return abonentAddress;
  }

  public void setAbonentAddress(String abonentAddress) {
    this.abonentAddress = abonentAddress;
  }

  public interface Receiver {
    public void execute(SecretUnregisterAbonentCmd cmd);
  }
}
