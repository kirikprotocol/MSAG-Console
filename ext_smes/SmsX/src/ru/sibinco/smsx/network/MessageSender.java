package ru.sibinco.smsx.network;

import com.logica.smpp.Data;
import ru.aurorisoft.smpp.*;
import ru.sibinco.smsx.InitializationException;
import ru.sibinco.smsx.utils.Service;
import ru.sibinco.smsx.utils.Utils;

import java.util.Properties;


// todo проверить интервалы посылки сообщений
public class MessageSender extends Service {

  private final static org.apache.log4j.Category Log = org.apache.log4j.Category.getInstance(MessageSender.class);

  private final SyncObject sendMonitor;
  private final OutgoingQueue outQueue;
  private final Multiplexor multiplexor;
  private final long syncSendTimeout;
  private final long throttledErrorDelay;
  private final long sendDelay;

  public MessageSender(final Properties config, OutgoingQueue outQueue, Multiplexor multiplexor) throws InitializationException {
    super(Log);
    setName("MessageSender");

    this.sendDelay = Utils.loadLong(config, "message.sender.send.delay");
    if (sendDelay < 0 || sendDelay > 60000)
      throw new InitializationException(getName() + ": wrong sendDelay");

    this.syncSendTimeout = Utils.loadLong(config, "message.sender.sync.send.timeout");
    if (syncSendTimeout < 0 || syncSendTimeout > 60000)
      throw new InitializationException(getName() + ": wrong syncSendTimeout");

    this.sendMonitor = new SyncObject();
    sendMonitor.setSyncMode(Utils.loadBoolean(config, "message.sender.sync.mode"));

    this.throttledErrorDelay = Utils.loadLong(config, "message.sender.throlled.error.delay");

    this.outQueue = outQueue;
    if (outQueue == null)
      throw new InitializationException(getName() + ": outgoing queue not specified");

    this.multiplexor = multiplexor;
    if (multiplexor == null)
      throw new InitializationException(getName() + ": multiplexor not specified");

  }

  public void  iterativeWork() {

    final OutgoingObject obj = outQueue.getOutgoingObject();

    try {
      final Message msg = obj.getOutgoingMessage();

      if (!validateMessage(msg))
        return;

      final Connector connector = lookupConnector(msg, multiplexor);

      // connector SHOULD be defined here !!!
      if (connector == null)
        throw new SMPPUserException(Data.ESME_RINVADR, "No route found for message: source #" + msg.getSourceAddress() + "; destination #" + msg.getDestinationAddress());


      multiplexor.assingSequenceNumber(msg, connector.getName());
      outQueue.messageSubmitted(obj);
      if (sendMonitor.isSyncMode()) {
        synchronized (sendMonitor) {
          sendMonitor.setConnId(msg.getConnectionId());
          sendMonitor.setSeqNum(msg.getSequenceNumber());
          sendMonitor.setLocked(true);
          Log.debug("Sending message in sync mode, address #" + msg.getSourceAddress() + "; abonent #" + msg.getDestinationAddress());
          multiplexor.sendMessage(obj.getOutgoingMessage(), connector.getName(), false);
          try {
            sendMonitor.wait(syncSendTimeout);
          } catch (InterruptedException e) {
            Log.error(getName() + " was interrupted: ", e);
          }
          if (sendMonitor.isLocked()) { // timeout !!!
            Log.warn("Syncronized send timeout reached, con=" + sendMonitor.getConnId() + ", sn=" + sendMonitor.getSeqNum());
            outQueue.updateOutgoingObject(sendMonitor.getConnId(), sendMonitor.getSeqNum());
          } else {
            Log.debug("Syncronized send completed, con=" + sendMonitor.getConnId() + ", sn=" + sendMonitor.getSeqNum() + ", status=" + sendMonitor.getStatus());
          }
          if (sendMonitor.getStatus() == Data.ESME_RTHROTTLED && throttledErrorDelay > 0) {
            try {
              sendMonitor.wait(throttledErrorDelay);
            } catch (InterruptedException e) {
              Log.warn(getName() + " was interrupted during throttled error delay.", e);
            }
          }
          sendMonitor.clean();
        }
      } else {
        Log.debug("Sending message in async mode, address #" + msg.getSourceAddress() + "; abonent #" + msg.getDestinationAddress());
        multiplexor.sendMessage(msg, connector.getName(), false);
      }
      if (sendDelay > 0) {
        synchronized(sendMonitor) {
          try {
            sendMonitor.wait(sendDelay);
          } catch (InterruptedException e) {
            Log.warn(getName() + " was interrupted during send delay.", e);
          }
        }
      }
    } catch (SMPPNotConnectedException e) {
      Log.warn(e.getMessage(), e);
    } catch (SMPPUserException e) {
      Log.warn("exc: " + e.getErrorCode(), e);
    } catch (SMPPException e) {
      Log.warn("exc:", e);
    }

  }

  public static boolean canSendMessage(final Message message, Multiplexor multiplexor) {
    return validateMessage(message) && lookupConnector(message, multiplexor) != null;
  }

  private static Connector lookupConnector(final Message message, Multiplexor multiplexor) {
//     if connector not defined try to lookup connector by name from outgoing message
    Connector connector = multiplexor.lookupConnectorByName(message.getConnectionName());
    if (connector != null)
      return connector;

//     if connector not defined try to lookup connector by source address of outgoing message
    final String connectorName = message.getSourceAddress();
    if (connectorName.lastIndexOf(":") > -1)
      return multiplexor.lookupConnectorByName(connectorName.substring(connectorName.lastIndexOf(":") + 1));


//     if connector not defined try to lookup connector by destination address
    if (message.getDestinationAddress() != null)
      return multiplexor.lookupConnector(message.getDestinationAddress());

    return null;
  }

  private static boolean validateMessage(final Message message) {
    return (message.getDestinationAddress() != null) && (message.getConnectionName() != null);
  }

  public SyncObject getSendMonitor() {
    return sendMonitor;
  }
}
