package ru.sibinco.smsx.engine.service.group.commands;

import ru.sibinco.smsx.engine.service.AsyncCommand;
import ru.sibinco.smsx.engine.service.group.DeliveryStatus;

/**
 * User: artem
 * Date: 02.12.2008
 */
public class GroupSendCommand extends AsyncCommand {

  public static final int ERR_UNKNOWN_OWNER = ERR_INT + 1;
  public static final int ERR_UNKNOWN_GROUP = ERR_INT + 2;
  public static final int ERR_UNKNOWN_SUBMITTER = ERR_INT + 3;

  // Init params
  private String submitter;
  private String message;
  private int destAddrSubunit;
  private String imsi;
  private String mscAddress;
  private boolean storable;

  // Results
  private DeliveryStatus[] deliveryStatuses;
  private int groupSize;

  public DeliveryStatus[] getDeliveryStatuses() {
    return deliveryStatuses;
  }

  public void setDeliveryStatuses(DeliveryStatus[] deliveryStatuses) {
    this.deliveryStatuses = deliveryStatuses;
  }

  public int getGroupSize() {
    return groupSize;
  }

  public void setGroupSize(int groupSize) {
    this.groupSize = groupSize;
  }

  public String getMessage() {
    return message;
  }

  public void setMessage(String message) {
    this.message = message;
  }

  public int getDestAddrSubunit() {
    return destAddrSubunit;
  }

  public void setDestAddrSubunit(int destAddrSubunit) {
    this.destAddrSubunit = destAddrSubunit;
  }

  public String getImsi() {
    return imsi;
  }

  public void setImsi(String imsi) {
    this.imsi = imsi;
  }

  public String getMscAddress() {
    return mscAddress;
  }

  public void setMscAddress(String mscAddress) {
    this.mscAddress = mscAddress;
  }

  public boolean isStorable() {
    return storable;
  }

  public void setStorable(boolean storable) {
    this.storable = storable;
  }

  public String getSubmitter() {
    return submitter;
  }

  public void setSubmitter(String submitter) {
    this.submitter = submitter;
  }
}
