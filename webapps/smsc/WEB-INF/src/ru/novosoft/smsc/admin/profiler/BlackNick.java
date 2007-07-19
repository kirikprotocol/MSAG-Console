package ru.novosoft.smsc.admin.profiler;

/**
 * User: artem
 * Date: 18.07.2007
 */

public class BlackNick {
  private String nick;

  public BlackNick(String nick) {
    this.nick = nick;
  }

  public BlackNick() {
  }

  public String getNick() {
    return nick;
  }

  public void setNick(String nick) {
    this.nick = nick;
  }
}
