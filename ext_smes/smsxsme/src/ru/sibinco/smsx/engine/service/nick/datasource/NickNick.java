package ru.sibinco.smsx.engine.service.nick.datasource;

/**
 * User: artem
 * Date: Sep 6, 2007
 */

public class NickNick {

  private String address;
  private String nick;

  public NickNick() {
  }

  public NickNick(String address, String nick) {
    this.address = address;
    this.nick = nick;
  }

  public String getAddress() {
    return address;
  }

  public void setAddress(String address) {
    this.address = address;
  }

  public String getNick() {
    return nick;
  }

  public void setNick(String nick) {
    this.nick = nick;
  }

}
