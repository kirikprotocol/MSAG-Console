package ru.sibinco.smsx.engine.smpphandler;

import com.eyeline.sme.handler.services.AbstractService;
import com.eyeline.sme.smpp.ShutdownedException;
import ru.aurorisoft.smpp.Message;

/**
 * User: artem
 * Date: 14.05.2008
 */

public abstract class AbstractSMPPService extends AbstractService {
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

  protected void sendMessage(String sourceAddress, String destinationAddress, String message, int destAddrSubunit) {
    try {
      final Message msg = new Message();
      msg.setSourceAddress(sourceAddress);
      msg.setDestinationAddress(destinationAddress);
      msg.setMessageString(message);
      msg.setDestAddrSubunit(destAddrSubunit);
      send(msg);
    } catch (ShutdownedException e) {
    }
  }
}
