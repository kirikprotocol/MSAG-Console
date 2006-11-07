package ru.sibinco.otasme.network;

import com.logica.smpp.Data;
import ru.aurorisoft.smpp.PDU;
import ru.aurorisoft.smpp.ResponseListener;
import ru.aurorisoft.smpp.SMPPException;

public class ResponseListenerImpl implements ResponseListener {

  private final static org.apache.log4j.Category Log = org.apache.log4j.Category.getInstance(ResponseListenerImpl.class);

  private SyncObject sendMonitor = new SyncObject();
  private OutgoingQueue messagesQueue;
  private IncomingQueue incomingQueue;

  public ResponseListenerImpl(OutgoingQueue messagesQueue, IncomingQueue incomingQueue) {
    this.messagesQueue = messagesQueue;
    this.incomingQueue = incomingQueue;
  }

  public void handleResponse(PDU pdu) throws SMPPException {
    String responseType = "SUBMIT_SM_RESP";
    switch(pdu.getType()) {
      case Data.ENQUIRE_LINK_RESP:
        responseType = "ENQUIRE_LINK_RESP";
        break;
      case Data.DATA_SM_RESP:
        responseType = "DATA_SM_RESP";
        break;
      case Data.WTS_RESPONSE:
        responseType = "WTS_RESPONSE";
        break;
    }

    Log.debug(responseType + " handled. Status #" + pdu.getStatus());
    if (pdu.getType() == Data.SUBMIT_SM_RESP || pdu.getType() == Data.DATA_SM_RESP) {
      if (sendMonitor.isSyncMode()) {
        if (sendMonitor.isLocked()) {
          Log.warn("Receiver locked by , con=" + sendMonitor.getConnId() + ", sn=" + sendMonitor.getSeqNum());
          if (pdu.getConnectionId() == sendMonitor.getConnId() && pdu.getSequenceNumber() == sendMonitor.getSeqNum()) {
            synchronized (sendMonitor) {
              try {
                sendMonitor.setLocked(false);
                sendMonitor.setStatus(pdu.getStatus());
                sendMonitor.notifyAll();
              } catch (Exception e) {
                Log.error("exc: ", e);
              }
            }
          } else {
            Log.warn("Received unexpected response, con=" + pdu.getConnectionId() + ", sn=" + pdu.getSequenceNumber());
          }
        } else {
          Log.warn("Sync mode is enabled, but no lock found, con=" + pdu.getConnectionId() + ", sn=" + pdu.getSequenceNumber());
        }
      }
      if (sendMonitor.isSyncMode()) {
        if (pdu.getStatus() == Data.ESME_RTHROTTLED) {
          incomingQueue.lockByThrottledError(true);
          messagesQueue.restoreOutgoingObject(pdu.getConnectionId(), pdu.getSequenceNumber());
        } else {
          incomingQueue.lockByThrottledError(false);
          if (pdu.getStatusClass() == PDU.STATUS_CLASS_TEMP_ERROR) {
            messagesQueue.updateOutgoingObject(pdu.getConnectionId(), pdu.getSequenceNumber());
          } else {
            messagesQueue.removeOutgoingObject(pdu.getConnectionId(), pdu.getSequenceNumber(), pdu.getStatusClass());
          }
        }
      } else {
        if (pdu.getStatusClass() == PDU.STATUS_CLASS_TEMP_ERROR) {
          messagesQueue.updateOutgoingObject(pdu.getConnectionId(), pdu.getSequenceNumber());
        } else {
          messagesQueue.removeOutgoingObject(pdu.getConnectionId(), pdu.getSequenceNumber(), pdu.getStatusClass());
        }
      }
    }
  }

  public void setSendMonitor(SyncObject sendMonitor) {
    this.sendMonitor = sendMonitor;
  }
}
