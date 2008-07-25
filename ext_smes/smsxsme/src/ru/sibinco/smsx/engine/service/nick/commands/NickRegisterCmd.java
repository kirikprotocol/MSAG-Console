package ru.sibinco.smsx.engine.service.nick.commands;

import ru.sibinco.smsx.engine.service.AsyncCommand;

/**
 * User: artem
 * Date: Sep 10, 2007
 */

public class NickRegisterCmd extends AsyncCommand {

  public static final int STATUS_INVALID_ABONENT_ADDRESS = STATUS_INT + 1;
  public static final int STATUS_INVALID_NICK = STATUS_INT + 2;
  public static final int STATUS_DELIVERED = STATUS_INT + 3;
  public static final int STATUS_MESSAGE_SENDED = STATUS_INT + 4;

  private String abonentAddress;
  private String nick;
  private int smppStatus;


  public String getAbonentAddress() {
    return abonentAddress;
  }

  public void setAbonentAddress(String abonentAddress) {
    this.abonentAddress = abonentAddress;
  }

  public String getNick() {
    return nick;
  }

  public void setNick(String nick) {
    this.nick = nick;
  }

  public int getSmppStatus() {
    return smppStatus;
  }

  public void setSmppStatus(int smppStatus) {
    this.smppStatus = smppStatus;
  }

  public interface Receiver {
    public void execute(NickRegisterCmd cmd);
  }

}
