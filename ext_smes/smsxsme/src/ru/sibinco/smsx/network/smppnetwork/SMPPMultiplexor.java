package ru.sibinco.smsx.network.smppnetwork;

import com.eyeline.sme.utils.config.ConfigException;
import com.eyeline.sme.utils.config.properties.PropertiesConfig;
import com.eyeline.sme.utils.worker.IterativeWorker;
import com.logica.smpp.Data;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.*;
import ru.sibinco.smsx.InitializationException;

import java.util.HashSet;
import java.util.Set;
import java.util.StringTokenizer;

/**
 * User: artem
 * Date: 25.04.2007
 */

public class SMPPMultiplexor implements ResponseListener, MessageListener {
  private static final Category log = Category.getInstance(SMPPMultiplexor.class);

  private Multiplexor transportMultiplexor;
  private final Set smscAddresses;

  private final SMPPOutgoingQueue outQueue;
  private final SMPPIncomingQueue inQueue;
  private final MessageSender messageSender;

  private final SyncObject sendMonitor = new SyncObject();
  private boolean checkEmptysourceaddr;
  private boolean checkBinary;
  private String smeAddress;
  private int maxRetries;
  private int retryInterval;

  public SMPPMultiplexor(final PropertiesConfig config) {
    try {
      smscAddresses = new HashSet();

      reloadProperties(config);

      transportMultiplexor = new Multiplexor();
//      transportMultiplexor = new SponsoredTestMultiplexor();
      transportMultiplexor.setMessageListener(this);
      transportMultiplexor.setResponseListener(this);
      transportMultiplexor.init(config, "");

      inQueue = new SMPPIncomingQueue();
      outQueue = new SMPPOutgoingQueue();

      messageSender = new MessageSender(config);

    } catch (SMPPException e) {
      throw new InitializationException(e);
    }
  }

  public SMPPMultiplexor(final PropertiesConfig config, Multiplexor transportMultiplexor) {
    try {
      smscAddresses = new HashSet();

      reloadProperties(config);

      this.transportMultiplexor = transportMultiplexor;
      this.transportMultiplexor.setMessageListener(this);
      this.transportMultiplexor.setResponseListener(this);
      this.transportMultiplexor.init(config, "");

      inQueue = new SMPPIncomingQueue();
      outQueue = new SMPPOutgoingQueue();

      messageSender = new MessageSender(config);

    } catch (SMPPException e) {
      throw new InitializationException(e);
    }
  }

  public void shutdown() throws SMPPException {
    log.warn("Shutdown called!");
    log.warn("Stop Message sender...");
    messageSender.stop();

    log.warn("Shutdown Transport multiplexor...");
    if (transportMultiplexor.isConnected())
      transportMultiplexor.shutdown();


    log.warn("SMPPMultiplexor shutdowned!");
  }

  public void connect() throws SMPPException {
    transportMultiplexor.connect();

    messageSender.start();
  }

  public void reloadProperties(final PropertiesConfig config) {
    try {

      smscAddresses.clear();
      final StringTokenizer st = new StringTokenizer(config.getString("smsc.addresses"), ",");
      while (st.hasMoreTokens())
        smscAddresses.add(st.nextToken());

      sendMonitor.setSyncMode(config.getBool("message.sender.sync.mode"));

      checkEmptysourceaddr = config.getBool("messagelistener.check.emptysourceaddr");
      checkBinary = config.getBool("messagelistener.check.binary");

      smeAddress = config.getString("sme.address");

      maxRetries = config.getInt("outgoing.queue.max.retries");
      retryInterval = config.getInt("outgoing.queue.retry.period");

      if (transportMultiplexor != null)
        transportMultiplexor.init(config, "");

      if (messageSender != null)
        messageSender.reloadProperties(config);

    } catch (ConfigException e) {
      throw new InitializationException(e);
    } catch (SMPPException e) {
      throw new InitializationException(e);
    }
  }

  public void handleResponse(PDU pdu) throws SMPPException {
    try {
      switch(pdu.getType()) {
        case Data.ENQUIRE_LINK_RESP:
          if (log.isDebugEnabled())
            log.debug("ENQUIRE_LINK_RESP handled. Status #" + pdu.getStatus());
          break;
        case Data.DATA_SM_RESP:
          if (log.isDebugEnabled())
            log.debug("DATA_SM_RESP handled. Status #" + pdu.getStatus());
          break;
        default:
          if (log.isDebugEnabled())
            log.debug("SUBMIT_SM_RESP handled. Status #" + pdu.getStatus());
      }

      if (pdu.getType() != Data.SUBMIT_SM_RESP && pdu.getType() != Data.DATA_SM_RESP)
        return;

      if (sendMonitor.isSyncMode()) {
        if (sendMonitor.isLocked()) {
          log.warn("Receiver locked by , con=" + sendMonitor.getConnId() + ", sn=" + sendMonitor.getSeqNum());

          if (pdu.getConnectionId() == sendMonitor.getConnId() && pdu.getSequenceNumber() == sendMonitor.getSeqNum()) {
            synchronized (sendMonitor) {
              try {
                sendMonitor.setLocked(false);
                sendMonitor.setStatus(pdu.getStatus());
                sendMonitor.notifyAll();
              } catch (Exception e) {
                log.error("exc: ", e);
              }
            }
          } else
            log.warn("Received unexpected response, con=" + pdu.getConnectionId() + ", sn=" + pdu.getSequenceNumber());

        } else
          log.warn("Sync mode is enabled, but no lock found, con=" + pdu.getConnectionId() + ", sn=" + pdu.getSequenceNumber());

        if (pdu.getStatus() == Data.ESME_RTHROTTLED) {
          if (log.isDebugEnabled())
            log.debug("OUT inserted in queue head by throttled error");
          outQueue.restoreOutgoingObject(pdu.getConnectionId(), pdu.getSequenceNumber());
          return;
        }
      }


      final SMPPTransportObject outObj = outQueue.removeOutgoingObject(pdu.getConnectionId(), pdu.getSequenceNumber());

//      if (pdu.getStatusClass() == PDU.STATUS_CLASS_PERM_ERROR || pdu.getStatus() == 1121) {
//        SMPPTransportObject outObj = outQueue.removeOutgoingObject(pdu.getConnectionId(), pdu.getSequenceNumber());
//        outObj.setStatus(pdu.getStatus() == 1121 ? SMPPTransportObject.STATUS_USSD_DIALOG_CLOSED : SMPPTransportObject.STATUS_CANT_SEND);
//        outObj.setCantSendReason(pdu.getStatus() == 1121 ? "Abonent close USSD dialog" : "Permanent SMPP error: " + pdu.getStatus());
//        outObj.setNextRepeatTime(0);
//        inQueue.putInObj(outObj);
      if (pdu.getStatusClass() == PDU.STATUS_CLASS_TEMP_ERROR) {
        log.error("Can't send message. Message will be repeated after " + retryInterval + " milliseconds");
        outObj.setNextTime(System.currentTimeMillis() + retryInterval);
        outQueue.addOutgoingObject(outObj);
      } else
        outObj.handleResponse(pdu);


    } catch (Throwable e) {
      log.error("Can't handle message response", e);
    }
  }

  public synchronized boolean handleMessage(Message msg) throws SMPPUserException {
    try {
      // if message from SMSC send delivery response and skip it
//      if (msg.isReceipt() || (smscAddresses != null && smscAddresses.contains(msg.getSourceAddress()))) {
//        log.debug("SKIP MSG FROM #" + msg.getSourceAddress() + ". REASON: " + (msg.isReceipt() ? "RECEIPT" : "SMSC"));
//        sendResponse(msg, Data.ESME_ROK);
//        return false;
//      }

      if (checkEmptysourceaddr) {
        final String source = msg.getSourceAddress();
        boolean skip = source == null || source.trim().equals("");
        if (!skip)
          skip = (source.lastIndexOf('.') >= 0 && source.lastIndexOf('.') + 1 == source.length()) ||
              (source.lastIndexOf('+') >= 0 && source.lastIndexOf('+') + 1 == source.length());
        if (skip) {
          if (log.isInfoEnabled())
            log.info("SKIP MSG FROM #" + msg.getSourceAddress() + ". REASON: empty Source Address");
          sendResponse(msg, Data.ESME_ROK);
          return false;
        }
      }

      if (checkBinary && msg.getEncoding() == Message.ENCODING_BINARY) {
        if (log.isInfoEnabled())
          log.info("SKIP MSG FROM #" + msg.getSourceAddress() + ". REASON: unsupported encoding: BINARY ");
        sendResponse(msg, Data.ESME_ROK);
        return true;
      }

      if (log.isDebugEnabled())
        log.debug("Received MSG, USSD #" + msg.getUssdServiceOp() + "; address #" + msg.getDestinationAddress() + "; abonent #" + msg.getSourceAddress()  + "; type: " + msg.getType() + "; msg: " + msg.getMessageString());

//      sendResponse(msg, Data.ESME_ROK);

      final SMPPTransportObject obj = new SMPPTransportObject();
      obj.setIncomingMessage(msg);
      inQueue.putInObj(obj);

      return true;
    } catch (Throwable e) { // logic exception (message format)
      log.error("Can't handle incoming message", e);
      sendResponse(msg, Data.ESME_RSYSERR);
      return false;
    }
  }



  public void sendResponse(final Message msg, final int status) {
    try {
      msg.setStatus(status);
      transportMultiplexor.sendResponse(msg);
      if (log.isDebugEnabled())
        log.debug("Delivery response sent, address #" + msg.getDestinationAddress() + "; abonent #" + msg.getSourceAddress() + "; status #" + msg.getStatus());
    } catch (Throwable e) {
      log.error("Exception occured sending delivery response.", e);
    }
  }



  public SMPPOutgoingQueue getOutQueue() {
    return outQueue;
  }

  public SMPPIncomingQueue getInQueue() {
    return inQueue;
  }





















  // MESSAGE SENDER

  private final class MessageSender extends IterativeWorker {

    private long syncSendTimeout;
    private long throttledErrorDelay;
    private long sendDelay;

    private MessageSender(final PropertiesConfig config) {
      super(log);
      reloadProperties(config);
    }

    public void reloadProperties(final PropertiesConfig config) {
      try {
        this.sendDelay = config.getLong("message.sender.send.delay");
        if (sendDelay < 0 || sendDelay > 60000)
          throw new InitializationException("Wrong sendDelay");

        this.syncSendTimeout = config.getLong("message.sender.sync.send.timeout");
        if (syncSendTimeout < 0 || syncSendTimeout > 60000)
          throw new InitializationException("Wrong syncSendTimeout");

        this.throttledErrorDelay = config.getLong("message.sender.throlled.error.delay");
      } catch (ConfigException e) {
        throw new InitializationException(e.getMessage());
      }
    }

    private void sendMessage(SMPPTransportObject outObj, Connector connector, boolean sendFlag) {
      if (outObj.getRepeatNumber() > maxRetries || (outObj.getOutgoingMessage().hasUssdServiceOp() && outObj.getRepeatNumber() > 1)) {
        if (log.isDebugEnabled())
          log.debug("OUT [CID=" + outObj.getOutgoingMessage().getConnectionId() + ";SN=" + outObj.getOutgoingMessage().getSequenceNumber() + "] finalized, max attempts value was reached");
        outObj.handleSendError();
        return;
      }

      outObj.setRepeatNumber(outObj.getRepeatNumber() + 1);
      outObj.setNextTime(System.currentTimeMillis() + retryInterval);

      outQueue.addOutgoingObject(outObj);
      try {
        transportMultiplexor.sendMessage(outObj.getOutgoingMessage(), connector.getName(), sendFlag);
      } catch (SMPPException e) {
        log.error("Can't send message. Message will be repeated after " + retryInterval + " milliseconds", e);
      }
    }

    protected void stopCurrentWork() {
      log.warn("Message sender stopped!!!!!!!!!");
      outQueue.notifyWaiters();
    }

    public void iterativeWork() {
      SMPPTransportObject obj = null;

      try {
        obj = outQueue.removeNextOutgoingObject();
        if (!isStarted())
          return;

        final Message msg = obj.getOutgoingMessage();

        if (msg.getSourceAddress() == null)
          msg.setSourceAddress(smeAddress);

        if (!validateMessage(msg))
          throw new SMPPUserException(Data.ESME_RINVADR, "Message has empty destination address and empty connection id");

        final Connector connector = lookupConnector(msg); // connector SHOULD be defined here !!!
        if (connector == null)
          throw new SMPPUserException(Data.ESME_RINVADR, "No subjects found for message: source #" + msg.getSourceAddress() + "; destination #" + msg.getDestinationAddress());

        transportMultiplexor.assingSequenceNumber(msg, connector.getName());

        if (sendMonitor.isSyncMode()) {
          synchronized (sendMonitor) {
            sendMonitor.setConnId(msg.getConnectionId());
            sendMonitor.setSeqNum(msg.getSequenceNumber());
            sendMonitor.setLocked(true);

            if (log.isDebugEnabled())
              log.debug("Sending message in sync mode, address #" + msg.getSourceAddress() + "; abonent #" + msg.getDestinationAddress());

            sendMessage(obj, connector, false);

            try {
              sendMonitor.wait(syncSendTimeout);
            } catch (InterruptedException e) {
              log.error("Interrupted: ", e);
            }

            if (sendMonitor.isLocked()) // timeout !!!
              log.warn("Syncronized send timeout reached, con=" + sendMonitor.getConnId() + ", sn=" + sendMonitor.getSeqNum());
            else if (log.isDebugEnabled())
              log.debug("Syncronized send completed, con=" + sendMonitor.getConnId() + ", sn=" + sendMonitor.getSeqNum() + ", status=" + sendMonitor.getStatus());

            if (sendMonitor.getStatus() == Data.ESME_RTHROTTLED && throttledErrorDelay > 0) {
              try {
                sendMonitor.wait(throttledErrorDelay);
              } catch (InterruptedException e) {
                log.warn("Interrupted during throttled error delay.", e);
              }
            }

            sendMonitor.clean();
          }

        } else {
          if (log.isDebugEnabled())
            log.debug("Sending message in async mode, address #" + msg.getSourceAddress() + "; abonent #" + msg.getDestinationAddress() + "; msg = " + msg.getMessageString());
          sendMessage(obj, connector, false);
        }

        if (sendDelay > 0) {
          synchronized(sendMonitor) {
            try {
              sendMonitor.wait(sendDelay);
            } catch (InterruptedException e) {
              log.warn("Interrupted during send delay.", e);
            }
          }
        }

      } catch (Throwable e) {
        log.error("Can't send message", e);
        if (obj != null)
          obj.handleSendError();
//        if (obj != null) {
//          obj.setStatus(SMPPTransportObject.STATUS_CANT_SEND);
//          obj.setCantSendReason("Internal error: " + e.getMessage());
//          inQueue.putInObj(obj);
//        }
      }

    }

    private Connector lookupConnector(final Message message) {
      // if connector not defined try to lookup connector by name from outgoing message
      final Connector connector = transportMultiplexor.lookupConnectorByName(message.getConnectionName());
      if (connector != null)
        return connector;

      // if connector not defined try to lookup connector by destination address
      if (message.getDestinationAddress() != null)
        return transportMultiplexor.lookupConnector(message.getDestinationAddress());

      // if connector not defined try to lookup connector by source address of outgoing message
      final String connectorName = message.getSourceAddress();
      if (connectorName.lastIndexOf(":") > -1)
        return transportMultiplexor.lookupConnectorByName(connectorName.substring(connectorName.lastIndexOf(":") + 1));

      return null;
    }

    private boolean validateMessage(final Message message) {
      return (message.getDestinationAddress() != null) && (message.getConnectionName() != null);
    }
  }

















  private final static class SyncObject {

    private boolean syncMode = false;
    private int connId = -1;
    private int seqNum = -1;
    private boolean locked = false;
    private int status = -1;

    public boolean isSyncMode() {
      return syncMode;
    }

    public void setSyncMode(boolean syncMode) {
      this.syncMode = syncMode;
    }

    public int getConnId() {
      return connId;
    }

    public void setConnId(int connId) {
      this.connId = connId;
    }

    public int getSeqNum() {
      return seqNum;
    }

    public void setSeqNum(int seqNum) {
      this.seqNum = seqNum;
    }

    public boolean isLocked() {
      return locked;
    }

    public void setLocked(boolean locked) {
      this.locked = locked;
    }

    public int getStatus() {
      return status;
    }

    public void setStatus(int status) {
      this.status = status;
    }

    public void clean() {
      setConnId(-1);
      setSeqNum(-1);
      setLocked(false);
      setStatus(-1);
    }
  }
}
