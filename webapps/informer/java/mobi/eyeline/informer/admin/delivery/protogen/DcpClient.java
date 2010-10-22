package mobi.eyeline.informer.admin.delivery.protogen;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.DeliveryException;
import mobi.eyeline.informer.admin.delivery.protogen.protocol.*;
import mobi.eyeline.protogen.framework.BufferReader;
import mobi.eyeline.protogen.framework.BufferWriter;
import mobi.eyeline.protogen.framework.ClientConnection;
import mobi.eyeline.protogen.framework.PDU;
import org.apache.log4j.Logger;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

/**
 * @author Aleksandr Khalitov
 */
class DcpClient extends ClientConnection {

  private static final Logger log = Logger.getLogger(DcpClient.class);

  private static final int RESPONSE_TIMEOUT = 5000;

  private final Map<Integer, ResponseListener> listeners = new HashMap<Integer, ResponseListener>();
  private boolean connected;

  public DcpClient(String host, int port) {
    Properties initProps = new Properties();
    initProps.setProperty("informer.host", host);
    initProps.setProperty("informer.port", port + "");
    init("informer", initProps);
  }

  @Override
  protected void onConnect() throws IOException {
    connected = true;
  }

  @Override
  protected void onSend(BufferWriter writer, PDU pdu) throws IOException {
    int pos = writer.getLength();
    writer.writeInt(0); // write 4 bytes for future length
    writer.writeInt(pdu.getTag());
    writer.writeInt(pdu.getSeqNum());
    pdu.encode(writer);
    writer.replaceInt(pos, writer.getLength() - pos - 4);
  }

  @Override
  protected PDU onReceive(BufferReader bufferReader) throws IOException {
    int tag = bufferReader.readInt();
    int seqNum = bufferReader.readInt();

    if (log.isDebugEnabled())
      log.debug("PDU received: tag=" + tag + ", seqNum=" + seqNum);

    ResponseListener l;
    synchronized (listeners) {
      l = listeners.get(seqNum);
    }
    if (l != null) {
      return l.receive(bufferReader, tag);
    } else {
      log.error("Unexpected seqNum=" + seqNum);
      throw new IOException("Unexpected seqNum=" + seqNum);
    }
  }

  @Override
  protected void handle(PDU pdu) {
  }

  <T extends PDU> T sendPdu(PDU request, T response) throws AdminException {
    ResponseListener l = new ResponseListener(response);
    int seq = request.assignSeqNum();
    if (response != null) {
      synchronized (listeners) {
        listeners.put(seq, l);
      }
    }
    try {
      send(request);
      if (log.isDebugEnabled())
        log.debug("Request sent: " + request);

      if (response != null) {
        PDU resp = l.getResponse(RESPONSE_TIMEOUT);
        if (resp != null) {
          if (log.isDebugEnabled())
            log.debug("Response received: " + response);
          if (resp.getTag() == DcpClientTag.FailResponse.getValue()) {
            FailResponse failResponse = (FailResponse) resp;
            log.error("Interaction error: " + failResponse.getStatus() + ", " + failResponse.getStatusMessage());
            throw new DeliveryException("interaction_error", failResponse.getStatus() + "");
          }
          return (T) resp;
        } else
          throw new DeliveryException("response_timeout");
      }

      return null;
    } catch (InterruptedException e) {
      throw new DeliveryException("request_interrupted");
    } catch (IOException e) {
      connected = false;
      throw new DeliveryException("interaction_error", e);
    }
  }


  public boolean isConnected() {
    return connected;
  }

  public void shutdown() {
    connected = false;
    super.shutdown();
  }


  AddDeliveryMessagesResp send(AddDeliveryMessages req) throws AdminException {
    return sendPdu(req, new AddDeliveryMessagesResp());
  }

  CreateDeliveryResp send(CreateDelivery req) throws AdminException {
    return sendPdu(req, new CreateDeliveryResp());
  }

  CountDeliveriesResp send(CountDeliveries req) throws AdminException {
    return sendPdu(req, new CountDeliveriesResp());
  }

  CountMessagesResp send(CountMessages req) throws AdminException {
    return sendPdu(req, new CountMessagesResp());
  }

  void send(UserAuth req) throws AdminException {
    sendPdu(req, new OkResponse());
  }

  protected GetUserStatsResp send(GetUserStats req) throws AdminException {
    return sendPdu(req, new GetUserStatsResp());
  }

  void send(ModifyDelivery req) throws AdminException {
    sendPdu(req, new OkResponse());
  }

  void send(DropDelivery req) throws AdminException {
    sendPdu(req, new OkResponse());
  }

  void send(ChangeDeliveryState req) throws AdminException {
    sendPdu(req, new OkResponse());
  }

  void send(DropDeliverymessages req) throws AdminException {
    sendPdu(req, new OkResponse());
  }

  GetDeliveryGlossaryResp send(GetDeliveryGlossary req) throws AdminException {
    return sendPdu(req, new GetDeliveryGlossaryResp());
  }

  void send(ModifyDeliveryGlossary req) throws AdminException {
    sendPdu(req, new OkResponse());
  }

  GetDeliveryStateResp send(GetDeliveryState req) throws AdminException {
    return sendPdu(req, new GetDeliveryStateResp());
  }

  GetDeliveryInfoResp send(GetDeliveryInfo req) throws AdminException {
    return sendPdu(req, new GetDeliveryInfoResp());
  }

  GetDeliveriesListResp send(GetDeliveriesList req) throws AdminException {
    return sendPdu(req, new GetDeliveriesListResp());
  }

  GetDeliveriesListNextResp send(GetDeliveriesListNext req) throws AdminException {
    return sendPdu(req, new GetDeliveriesListNextResp());
  }

  RequestMessagesStateResp send(RequestMessagesState req) throws AdminException {
    return sendPdu(req, new RequestMessagesStateResp());
  }

  GetNextMessagesPackResp send(GetNextMessagesPack req) throws AdminException {
    return sendPdu(req, new GetNextMessagesPackResp());
  }


  private static class ResponseListener {
    private final CountDownLatch respLatch = new CountDownLatch(1);

    private PDU response;
    private final PDU responseEx;

    ResponseListener(PDU responseEx) {
      this.responseEx = responseEx;
    }

    PDU getResponse(int timeout) throws InterruptedException {
      respLatch.await(timeout, TimeUnit.MILLISECONDS);
      return response;
    }

    int getExpectedResponseTag() {
      return responseEx.getTag();
    }

    PDU receive(BufferReader buffer, int tag) throws IOException {
      if (tag == DcpClientTag.FailResponse.getValue()) {
        FailResponse failResponse = new FailResponse();
        failResponse.decode(buffer);
        response = failResponse;
      } else if (getExpectedResponseTag() != tag) {
        log.error("Unexpected tag: " + tag);
        throw new IOException("Unexpected tag: " + tag);
      } else {
        responseEx.decode(buffer);
        response = responseEx;
      }
      respLatch.countDown();
      return response;
    }

  }

}
