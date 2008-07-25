package ru.sibinco.smsx.engine.service.blacklist.commands;

import ru.sibinco.smsx.engine.service.AsyncCommand;
import ru.sibinco.smsx.engine.service.Command;
import ru.sibinco.smsx.engine.service.CommandExecutionException;

/**
 * User: artem
 * Date: Sep 10, 2007
 */

public class BlackListAddCmd extends Command {

  private String msisdn;

  public String getMsisdn() {
    return msisdn;
  }

  public void setMsisdn(String msisdn) {
    this.msisdn = msisdn;
  }

  public interface Receiver {
    public void execute(BlackListAddCmd cmd) throws CommandExecutionException;
  }
}
