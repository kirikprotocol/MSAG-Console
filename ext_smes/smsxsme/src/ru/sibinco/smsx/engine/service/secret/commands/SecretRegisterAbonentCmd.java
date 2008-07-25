package ru.sibinco.smsx.engine.service.secret.commands;

import ru.sibinco.smsx.engine.service.Command;
import ru.sibinco.smsx.engine.service.CommandExecutionException;

/**
 * User: artem
 * Date: Sep 11, 2007
 */

public class SecretRegisterAbonentCmd extends Command {

  public static final int ERR_SOURCE_ABONENT_ALREADY_REGISTERED = ERR_INT + 1;

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
    public void execute(SecretRegisterAbonentCmd cmd) throws CommandExecutionException;
  }
}
