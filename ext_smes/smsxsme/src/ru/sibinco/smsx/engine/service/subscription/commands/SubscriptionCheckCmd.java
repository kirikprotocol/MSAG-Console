package ru.sibinco.smsx.engine.service.subscription.commands;

import ru.sibinco.smsx.engine.service.Command;
import ru.sibinco.smsx.engine.service.CommandExecutionException;

/**
 * User: artem
 * Date: 28.07.2008
 */

public class SubscriptionCheckCmd extends Command {

  public static final int ERR_INV_MSISDN = ERR_INT + 1;

  private String address;

  public String getAddress() {
    return address;
  }

  public void setAddress(String address) {
    this.address = address;
  }

  public interface Receiver {
    public boolean execute(SubscriptionCheckCmd cmd) throws CommandExecutionException;
  }
}
