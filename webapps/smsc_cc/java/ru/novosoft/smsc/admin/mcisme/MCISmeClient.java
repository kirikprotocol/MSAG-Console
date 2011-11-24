package ru.novosoft.smsc.admin.mcisme;

import mobi.eyeline.protogen.framework.BufferReader;
import mobi.eyeline.protogen.framework.BufferWriter;
import mobi.eyeline.protogen.framework.ClientConnection;
import mobi.eyeline.protogen.framework.PDU;
import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.mcisme.protocol.*;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

/**
 * author: Aleksandr Khalitov
 */
@SuppressWarnings({"TypeMayBeWeakened", "OverlyCoupledClass"})
class MCISmeClient extends ClientConnection{

  private static final Logger log = Logger.getLogger(MCISmeClient.class);

  private static final int RESPONSE_TIMEOUT = 5000;

  private final Map<Integer, ResponseListener> listeners = new HashMap<Integer, ResponseListener>();

  private boolean connected;

  MCISmeClient(String host, int port) {
    Properties initProps = new Properties();
    initProps.setProperty("mci.host", host);
    initProps.setProperty("mci.port", Integer.toString(port));
    init("mci", initProps);
  }

  @Override
  protected void onConnect() throws IOException {
    if (log.isDebugEnabled())
      log.debug("Connected to MCISme. Sending register request.");
    connected = true;
  }

  @Override
  protected void onSend(BufferWriter writer, PDU pdu) throws IOException {
    MCAClientProtocol.encodeMessage(writer, pdu);
    if( log.isDebugEnabled() ) log.debug("PDU sent: "+pdu);
  }

  @Override
  protected PDU onReceive(BufferReader bufferReader) throws IOException {
    PDU pdu = MCAClientProtocol.decodeMessage(bufferReader);

    ResponseListener l;
    synchronized(listeners) {
      l = listeners.get(pdu.getSeqNum());
    }
    if (l != null) {
      if (l.getExpectedResponseTag() != pdu.getTag()) {
        throw new IOException("PDU rcvd unexpected tag: " + pdu);
      }
      if (log.isDebugEnabled()) log.debug("PDU rcvd: "+pdu);
      return l.receive(pdu);
    } else {
      throw new IOException("PDU rcvd unexpected seqnum: " + pdu);
    }
  }

  @Override
  protected void handle(PDU pdu) {
  }

  @SuppressWarnings({"unchecked"})
  private <T extends PDU> T sendPdu(PDU request, T response) throws AdminException {
    ResponseListener l = null;
    int seq = request.assignSeqNum();
    if (response != null) {
      l = new ResponseListener(response);
      synchronized (listeners) {
        listeners.put(seq, l);
      }
    }
    try {
      send(request);
      if (l != null) {
        PDU resp = l.getResponse(RESPONSE_TIMEOUT);
        if (resp != null) {
          return (T) resp;
        } else
          throw new MCISmeException("response_timeout");
      }

      return null;
    } catch (InterruptedException e) {
      throw new MCISmeException("request_interrupted");
    } catch (IOException e) {
      connected = false;
      throw new MCISmeException("interaction_error", e);
    }
  }


  public boolean isConnected() {
    return connected;
  }

  public GetStatsResp send(GetStats req) throws AdminException {
    return sendPdu(req, new GetStatsResp());
  }

  public GetRunStatsResp send(GetRunStats req) throws AdminException {
    return sendPdu(req, new GetRunStatsResp());
  }

  public FlushStatsResp send(FlushStats req) throws AdminException {
    return sendPdu(req, new FlushStatsResp());
  }

  public GetProfileResp send(GetProfile req) throws AdminException {
    return sendPdu(req, new GetProfileResp());
  }

  public SetProfileResp send(SetProfile req) throws AdminException {
    return sendPdu(req, new SetProfileResp());
  }

  public GetSchedResp send(GetSched req) throws AdminException {
    return sendPdu(req, new GetSchedResp());
  }

  public GetSchedListResp send(GetSchedList req) throws AdminException {
    return sendPdu(req, new GetSchedListResp());
  }


  // Access Controll List


  private static class ResponseListener {
    private final CountDownLatch respLatch = new CountDownLatch(1);

    private PDU response;
    private final int responseTag;

    public ResponseListener(PDU responseEx) {
      this.responseTag = responseEx.getTag();
    }

    public PDU getResponse(int timeout) throws InterruptedException {
      respLatch.await(timeout, TimeUnit.MILLISECONDS);
      return response;
    }

    public int getExpectedResponseTag() {
      return responseTag;
    }

    public PDU receive(PDU resp) throws IOException {
      response = resp;
      respLatch.countDown();
      return response;
    }

  }
}
