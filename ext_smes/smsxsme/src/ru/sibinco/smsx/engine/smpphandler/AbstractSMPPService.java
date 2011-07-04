package ru.sibinco.smsx.engine.smpphandler;

import com.eyeline.sme.handler.services.AbstractService;
import com.eyeline.sme.smpp.ShutdownedException;
import ru.aurorisoft.smpp.Message;

/**
 * User: artem
 * Date: 14.05.2008
 */

public abstract class AbstractSMPPService extends AbstractService {

  protected void reply(Message m, String sourceAddress, String text) {
    try {
      final Message msg = new Message();
      msg.setSourceAddress(m.getDestinationAddress());
      msg.setDestinationAddress(sourceAddress);
      msg.setMessageString(text);
      msg.setConnectionName(m.getConnectionName());
      send(msg);
    } catch (ShutdownedException e) {
    }
  }

  protected void reply(Message initialMessage, String text) {
    try {
      final Message msg = new Message();
      msg.setSourceAddress(initialMessage.getDestinationAddress());
      msg.setDestinationAddress(initialMessage.getSourceAddress());
      msg.setMessageString(text);
      msg.setConnectionName(initialMessage.getConnectionName());
      send(msg);
    } catch (ShutdownedException e) {
    }
  }

  protected void sendMessage(String sourceAddress, String destinationAddress, String message) {
    try {
      final Message msg = new Message();
      msg.setSourceAddress(sourceAddress);
      msg.setDestinationAddress(destinationAddress);
      msg.setMessageString(message);
      send(msg);
    } catch (ShutdownedException e) {
    }
  }

  protected void sendMessage(String sourceAddress, String destinationAddress, String connectionName, String message) {
    try {
      final Message msg = new Message();
      msg.setSourceAddress(sourceAddress);
      msg.setDestinationAddress(destinationAddress);
      msg.setMessageString(message);
      msg.setConnectionName(connectionName);
      send(msg);
    } catch (ShutdownedException e) {
    }
  }

  protected void sendMessage(String sourceAddress, String destinationAddress, String connectionName, String message,  int destAddrSubunit) {
    try {
      final Message msg = new Message();
      msg.setSourceAddress(sourceAddress);
      msg.setDestinationAddress(destinationAddress);
      msg.setMessageString(message);
      msg.setConnectionName(connectionName);
      msg.setDestAddrSubunit(destAddrSubunit);
      send(msg);
    } catch (ShutdownedException e) {
    }
  }
}
