package ru.sibinco.smsx.engine.service.group.commands;

import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.Command;

/**
 * User: artem
 * Date: 16.07.2008
 */

public class GroupDeliveryReportCmd extends Command {

  private String address;
  private boolean delivered;
  private int umr;

  public String getAddress() {
    return address;
  }

  public void setAddress(String address) {
    this.address = address;
  }

  public boolean isDelivered() {
    return delivered;
  }

  public void setDelivered(boolean delivered) {
    this.delivered = delivered;
  }

  public int getUmr() {
    return umr;
  }

  public void setUmr(int umr) {
    this.umr = umr;
  }  

  public interface Receiver {
    public void execute(GroupDeliveryReportCmd cmd) throws CommandExecutionException;
  }
}
