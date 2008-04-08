package ru.sibinco.smsx.engine.smpphandler;

import com.logica.smpp.Data;
import ru.aurorisoft.smpp.Message;
import ru.sibinco.smsx.network.smppnetwork.SMPPMultiplexor;
import ru.sibinco.smsx.network.smppnetwork.SMPPTransportObject;

/**
 * User: artem
 * Date: 29.06.2007
 */

public abstract class SMPPHandler {

  private SMPPHandler nextHandler;

  protected final SMPPMultiplexor multiplexor;

  protected SMPPHandler(SMPPMultiplexor multiplexor) {
    this.multiplexor = multiplexor;
  }

  public void setNextHandler(SMPPHandler nextHandler) {
    this.nextHandler = nextHandler;
  }

  public void handleIncomingObject(SMPPTransportObject inObj) {
    if (handleInObj(inObj))
      return;

    if (nextHandler != null)
      nextHandler.handleIncomingObject(inObj);
    else if (inObj.getIncomingMessage() != null)
      sendResponse(inObj.getIncomingMessage(), Data.ESME_RX_P_APPN);
  }

  protected void sendResponse(Message msg, int status) {
    multiplexor.sendResponse(msg, status);
  }

  protected void sendMessage(String sourceAddress, String destinationAddress, String message, int destAddrSubunit) {
    final Message msg = new Message();
    msg.setSourceAddress(sourceAddress);
    msg.setDestinationAddress(destinationAddress);
    msg.setMessageString(message);
    msg.setDestAddrSubunit(destAddrSubunit);
    sendMessage(msg);
  }

  protected void sendMessage(String sourceAddress, String destinationAddress, String message) {
    final Message msg = new Message();
    msg.setSourceAddress(sourceAddress);
    msg.setDestinationAddress(destinationAddress);
    msg.setMessageString(message);
    sendMessage(msg);
  }

  protected void sendMessage(Message msg) {
    final SMPPTransportObject outObj = new SMPPTransportObject();
    outObj.setOutgoingMessage(msg);
    sendMessage(outObj);
  }

  protected void sendMessage(SMPPTransportObject outObj) {
    multiplexor.getOutQueue().addOutgoingObject(outObj);
  }

  protected abstract boolean handleInObj(SMPPTransportObject inObj);
}
