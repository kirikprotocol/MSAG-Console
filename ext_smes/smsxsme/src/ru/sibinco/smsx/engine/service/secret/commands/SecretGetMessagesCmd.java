package ru.sibinco.smsx.engine.service.secret.commands;

import ru.sibinco.smsx.engine.service.Command;

/**
 * User: artem
 * Date: Sep 11, 2007
 */

public class SecretGetMessagesCmd extends Command {
  public static final int STATUS_SOURCE_ABONENT_NOT_REGISTERED = STATUS_INT + 1;
  public static final int STATUS_NO_MESSAGES = STATUS_INT + 2;
  public static final int STATUS_INVALID_PASSWORD = STATUS_INT + 3;

  private String abonentAddress;
  private String password;

  public String getAbonentAddress() {
    return abonentAddress;
  }

  public void setAbonentAddress(String abonentAddress) {
    this.abonentAddress = abonentAddress;
  }

  public String getPassword() {
    return password;
  }

  public void setPassword(String password) {
    this.password = password;
  }

  public interface Receiver {
    public void execute(SecretGetMessagesCmd cmd);
  }
}
