package ru.sibinco.otasme.network;

import com.logica.smpp.Data;
import ru.aurorisoft.smpp.*;
import ru.sibinco.otasme.InitializationException;

import java.util.Set;

public class MessageListenerImpl implements MessageListener {
  private final static org.apache.log4j.Category Log = org.apache.log4j.Category.getInstance(MessageListenerImpl.class);

  private Multiplexor multiplexor;
  private IncomingQueue inQueue;
  private Set smscAddresses = null;

  public MessageListenerImpl(Multiplexor multiplexor,
                             IncomingQueue inQueue,
                             Set smscAddresses) throws InitializationException {
    if (multiplexor == null)
      throw new InitializationException("Could not construct MessageListener with NULL Multiplexor.");
    this.multiplexor = multiplexor;
    if (inQueue == null)
      throw new InitializationException("Could not construct MessageListener with NULL IncomingQueue.");
    this.inQueue = inQueue;
    this.smscAddresses = smscAddresses;
    Log.info("MessageListener created.");
  }

  public boolean handleMessage(Message msg) throws SMPPUserException {
    try {
      // if message from SMSC send delivery response and skip it
      if (msg.isReceipt() || (smscAddresses != null && smscAddresses.contains(msg.getSourceAddress()))) {
        Log.debug("SKIP MSG FROM #" + msg.getSourceAddress() + ". REASON: " + (msg.isReceipt() ? "RECEIPT" : "SMSC"));
        sendResponse(msg, Data.ESME_ROK);
        return false;
      }

      if (msg.getSourceAddress() == null || msg.getSourceAddress().trim().equals("")) {
        Log.info("SKIP MSG FROM #" + msg.getSourceAddress() + ". REASON: empty Source Address");
        sendResponse(msg, Data.ESME_ROK);
        return false;
      }
      
      if (msg.getEncoding() == Message.ENCODING_BINARY) {
        Log.info("SKIP MSG FROM #" + msg.getSourceAddress() + ". REASON: unsupported encoding: BINARY ");
        sendResponse(msg, Data.ESME_ROK);
        return true;
      }

      Log.debug("Received MSG, USSD #" + msg.getUssdServiceOp() + "; address #" + msg.getDestinationAddress() + "; abonent #" + msg.getSourceAddress()  + "; type: " + msg.getType() + "; msg: " + msg.getMessageString());
      IncomingObject incomingObject = new IncomingObject(msg);
      try {
        inQueue.checkQueueLength(incomingObject);
      } catch (QueueOverflowException e) {
        Log.error("Queue full.", e);
        sendResponse(msg, Data.ESME_RMSGQFUL);
        return true;
      } catch (UserQueueOverflowException e) {
        Log.error("User queue full.", e);
        sendResponse(msg, Data.ESME_RMSGQFUL);
        return true;
      }
       sendResponse(msg, Data.ESME_ROK);
      inQueue.addIncomingObject(incomingObject);
      return true;
    } catch (Exception e) { // logic exception (message format)
      Log.error("Cannot handle MSG", e);
      sendResponse(msg, Data.ESME_RSYSERR);
      return false;
    }
  }

  private void sendResponse(Message msg, int status) {
    try {
      msg.setStatus(status);
      multiplexor.sendResponse(msg);
      Log.debug("Delivery response sent, address #" + msg.getDestinationAddress() + "; abonent #" + msg.getSourceAddress() + "; status #" + msg.getStatus());
    } catch (SMPPException e) {
      Log.warn("Exception occured sending delivery response.", e);
    }
  }
}
