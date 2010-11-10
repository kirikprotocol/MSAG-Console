package mobi.eyeline.informer.admin.infosme.protogen;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.infosme.InfosmeException;
import mobi.eyeline.informer.admin.infosme.protogen.protocol.*;
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
class InfosmeClient extends ClientConnection {

  private static final Logger log = Logger.getLogger(InfosmeClient.class);

  private static final int RESPONSE_TIMEOUT = 5000;

  private final Map<Integer, ResponseListener> listeners = new HashMap<Integer, ResponseListener>();
  private boolean connected;

  InfosmeClient(String host, int port) {
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
    synchronized(listeners) {
      l = listeners.get(seqNum);
    }
    if (l != null) {
      if (l.getExpectedResponseTag() != tag) {
        log.error("Unexpected tag: " + tag + " for seqNum: " + seqNum);
        throw new IOException("Unexpected tag: " + tag + " for seqNum: " + seqNum);
      }

      return l.receive(bufferReader);
    } else {
      log.error("Unexpected seqNum=" + seqNum);
      throw new IOException("Unexpected seqNum=" + seqNum);
    }
  }

  @Override
  protected void handle(PDU pdu) {
  }

  private <T extends PDU> T sendPdu(PDU request, T response) throws AdminException {
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
          return (T) resp;
        } else
          throw new InfosmeException("response_timeout");
      }

      return null;
    } catch (InterruptedException e) {
      throw new InfosmeException("request_interrupted");
    } catch (IOException e) {
      connected = false;
      throw new InfosmeException("interaction_error", e);
    }
  }


  protected boolean isConnected() {
    return connected;
  }

  
  protected ConfigOpResult send(ConfigOp req) throws AdminException{
    return sendPdu(req, new ConfigOpResult());     
  }
  
  protected LoggerGetCategoriesResp send(LoggerGetCategories req) throws AdminException{
    return sendPdu(req, new LoggerGetCategoriesResp());
  }
  
  protected LoggerSetCategoriesResp send(LoggerSetCategories req) throws AdminException {
    return sendPdu(req, new LoggerSetCategoriesResp());
  }
  
  protected SetDefaultSmscResp send(SetDefaultSmsc req) throws AdminException {
    return sendPdu(req, new SetDefaultSmscResp());
  }

  protected SendTestSmsResp send(SendTestSms req) throws AdminException {
    return sendPdu(req, new SendTestSmsResp());
  }

  public void shutdown() {
    connected = false;
    super.shutdown();
  }



  private static class ResponseListener {
    private final CountDownLatch respLatch = new CountDownLatch(1);

    private PDU response;
    private PDU responseEx;

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

    PDU receive(BufferReader buffer) throws IOException {
      responseEx.decode(buffer);
      response = responseEx;
      respLatch.countDown();
      return response;
    }

  }
}
