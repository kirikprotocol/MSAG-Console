package ru.sibinco.smsx.engine.service.blacklist.commands;

import ru.sibinco.smsx.engine.service.Command;

/**
 * User: artem
 * Date: Sep 10, 2007
 */

public class BlackListRemoveCmd extends Command {
  private String msisdn;

  public String getMsisdn() {
    return msisdn;
  }

  public void setMsisdn(String msisdn) {
    this.msisdn = msisdn;
  }

  public interface Receiver {
    public void execute(BlackListRemoveCmd cmd);
  }
}
