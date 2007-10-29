package ru.sibinco.otasme.network.smpp;

import com.eyeline.sme.utils.queue.delayed.DelayedObject;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.PDU;

public class SMPPTransportObject extends DelayedObject {

  public static final int STATUS_OK = 0;
  public static final int STATUS_CANT_SEND = 1;
  public static final int STATUS_USSD_DIALOG_CLOSED = 2;

  private int status = STATUS_OK;
  private Message incomingMessage = null;
  private Message outgoingMessage = null;
  private String id;
  private String cantSendReason;
  private int repeatNumber;

  public Message getOutgoingMessage() {
    return outgoingMessage;
  }

  public void setOutgoingMessage(Message outgoingMessage) {
    this.outgoingMessage = outgoingMessage;
  }

  public void setIncomingMessage(final Message incominMessage) {
    this.incomingMessage = incominMessage;
  }

  public Message getIncomingMessage() {
    return incomingMessage;
  }

  public int getStatus() {
    return status;
  }

  public void setStatus(int status) {
    this.status = status;
  }

  public String getId() {
    return id;
  }

  public void setId(String id) {
    this.id = id;
  }

  public String getCantSendReason() {
    return cantSendReason;
  }

  public void setCantSendReason(String cantSendReason) {
    this.cantSendReason = cantSendReason;
  }

  public void handleResponse(PDU response) {
  }

  public void handleSendError() {
  }

  public int getRepeatNumber() {
    return repeatNumber;
  }

  public void setRepeatNumber(int repeatNumber) {
    this.repeatNumber = repeatNumber;
  }
}
