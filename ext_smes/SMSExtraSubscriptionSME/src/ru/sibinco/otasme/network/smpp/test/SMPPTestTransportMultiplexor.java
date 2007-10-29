package ru.sibinco.otasme.network.smpp.test;

import ru.aurorisoft.smpp.*;
import org.apache.log4j.Category;

import java.util.Properties;

import com.logica.smpp.Data;

/**
 * User: artem
 * Date: Oct 8, 2007
 */

public class SMPPTestTransportMultiplexor extends Multiplexor{

  private static final Category log = Category.getInstance(SMPPTestTransportMultiplexor.class);

  public MessageListener messageListener;
  public ResponseListener responseListener;

  private final Connector connector = new ConnectorImpl();

  private int seqId = 0;

  public void init(Properties config, String prefix) throws SMPPException {
  }

  public void setMessageListener(MessageListener messageListener) {
    this.messageListener = messageListener;
  }

  public void setResponseListener(ResponseListener responseListener) {
    this.responseListener = responseListener;
  }

  public void sendResponse(Message message) {
  }

  public void sendResponse(PDU response) {
  }

  public void shutdown() {
  }


  public void assingSequenceNumber(Message message, String connectorName) {
    seqId++;
    message.setSequenceNumber(seqId);
  }

  public void handleMessage(Message message) {
    log.debug("DELIVER_SM: src=" + message.getSourceAddress() + "; dst=" + message.getDestinationAddress() + "; data: " + message.getMessageString());
    try {
      messageListener.handleMessage(message);
    } catch (SMPPUserException e) {
      e.printStackTrace();
    }
  }

  public void sendMessage(Message message, String connectorName, boolean flag) throws SMPPException {
    responseListener.handleResponse(new MySubmitResponse(message));
  }

  public void connect() throws SMPPException {
  }

  public Connector lookupConnectorByName(String name) {
    return connector;
  }

  public Connector lookupConnector(String name) {
    return connector;
  }







  protected static class MySubmitResponse implements PDU {

    private final Message message;
    private final int status;
    private final int statusClass;

    public MySubmitResponse(Message message) {
      this.message = message;
      this.status = Data.ESME_ROK;
      this.statusClass = Message.STATUS_CLASS_NO_ERROR;
    }

    public MySubmitResponse(Message message, int status, int statusClass) {
      this.message = message;
      this.status = status;
      this.statusClass = statusClass;
    }

    public int getType() {
      return Data.SUBMIT_SM_RESP;
    }

    public void setType(int i) {

    }

    public int getSequenceNumber() {
      return message.getSequenceNumber();
    }

    public void setSequenceNumber(int i) {

    }

    public int getConnectionId() {
      return message.getConnectionId();
    }

    public int getStatus() {
      return status;
    }

    public void setStatus(int i) {
    }

    public int getStatusClass() {
      return statusClass;
    }

    public void setStatusClass(int i) {
    }

    public boolean isResponse() {
      return false;
    }
  }

}
