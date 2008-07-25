package ru.sibinco.smsx.engine.service.secret.commands;

import ru.sibinco.smsx.engine.service.Command;
import ru.sibinco.smsx.engine.service.CommandExecutionException;

/**
 * User: artem
 * Date: Sep 11, 2007
 */

public class SecretChangePasswordCmd extends Command {
  public static final int ERR_SOURCE_ABONENT_NOT_REGISTERED = ERR_INT + 1;
  public static final int ERR_INVALID_PASSWORD = ERR_INT + 2;

  private String abonentAddress;
  private String oldPassword;
  private String newPassword;

  public String getAbonentAddress() {
    return abonentAddress;
  }

  public void setAbonentAddress(String abonentAddress) {
    this.abonentAddress = abonentAddress;
  }

  public String getOldPassword() {
    return oldPassword;
  }

  public void setOldPassword(String oldPassword) {
    this.oldPassword = oldPassword;
  }

  public String getNewPassword() {
    return newPassword;
  }

  public void setNewPassword(String newPassword) {
    this.newPassword = newPassword;
  }

  public interface Receiver {
    public void execute(SecretChangePasswordCmd cmd) throws CommandExecutionException;
  }
}
