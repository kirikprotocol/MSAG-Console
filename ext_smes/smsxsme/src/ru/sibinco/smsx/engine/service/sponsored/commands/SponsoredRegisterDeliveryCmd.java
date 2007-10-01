package ru.sibinco.smsx.engine.service.sponsored.commands;

import ru.sibinco.smsx.engine.service.Command;

/**
 * User: artem
 * Date: Sep 11, 2007
 */

public class SponsoredRegisterDeliveryCmd extends Command {

  private String destinationAddress;

  public String getDestinationAddress() {
    return destinationAddress;
  }

  public void setDestinationAddress(String destinationAddress) {
    this.destinationAddress = destinationAddress;
  }

  public interface Receiver {
    public void execute(SponsoredRegisterDeliveryCmd cmd);
  }
}
