package ru.sibinco.smsx.engine.service.blacklist.commands;

import ru.sibinco.smsx.engine.service.Command;

/**
 * User: artem
 * Date: Sep 10, 2007
 */

public class BlackListCheckMsisdnCmd extends Command {

  private String msisdn;
  private boolean inBlackList;

  public String getMsisdn() {
    return msisdn;
  }

  public void setMsisdn(String msisdn) {
    this.msisdn = msisdn;
  }

  public boolean isInBlackList() {
    return inBlackList;
  }

  public void setInBlackList(boolean inBlackList) {
    this.inBlackList = inBlackList;
  }

  public interface Receiver {
    public void execute(BlackListCheckMsisdnCmd cmd);
  }
}
