package ru.sibinco.smsx.engine.service.nick.commands;

import ru.sibinco.smsx.engine.service.AsyncCommand;

/**
 * User: artem
 * Date: Sep 13, 2007
 */

public class NickUnregisterCmd extends AsyncCommand {

  public static final int STATUS_DELIVERED = STATUS_INT + 1;

  private String abonentAddress;
  private int smppStatus;

  public String getAbonentAddress() {
    return abonentAddress;
  }

  public void setAbonentAddress(String abonentAddress) {
    this.abonentAddress = abonentAddress;
  }

  public int getSmppStatus() {
    return smppStatus;
  }

  public void setSmppStatus(int smppStatus) {
    this.smppStatus = smppStatus;
  }

  public interface Receiver {
    public void execute(NickUnregisterCmd cmd);
  }
}
