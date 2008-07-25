package ru.sibinco.smsx.engine.service.group.commands;

import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.Command;

/**
 * User: artem
 * Date: 16.07.2008
 */

public class GroupDeliveryReportCmd extends Command {

  private DeliveryStatus deliveryStatus;
  private String owner;
  private int umr;

  public DeliveryStatus getDeliveryStatus() {
    return deliveryStatus;
  }

  public void setDeliveryStatus(DeliveryStatus deliveryStatus) {
    this.deliveryStatus = deliveryStatus;
  }

  public String getOwner() {
    return owner;
  }

  public void setOwner(String owner) {
    this.owner = owner;
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

  public enum DeliveryStatus {
    DELIVERED, LIST_NOT_FOUND, OWNER_NOT_FOUND, ACCESS_DENIED, SYS_ERR
  }
}
