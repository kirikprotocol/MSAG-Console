package mobi.eyeline.informer.admin.protogen;

import mobi.eyeline.protogen.framework.BufferReader;
import mobi.eyeline.protogen.framework.BufferWriter;
import mobi.eyeline.protogen.framework.PDU;
import org.apache.log4j.Logger;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * @author Artem Snopkov
 */
public abstract class SyncProtogenConnection {

  private static final Logger logger = Logger.getLogger(SyncProtogenConnection.class);

  private final Lock sendLock = new ReentrantLock();

  private final String host;
  private final int port;
  private final int timeout;

  private Socket socket;
  private InputStream is;
  private OutputStream os;

  protected SyncProtogenConnection(String host, int port, int timeout) {
    this.host = host;
    this.port = port;
    this.timeout = timeout;
  }

  private static void serialize(PDU request, OutputStream os) throws IOException {
    BufferWriter buffer = new BufferWriter();
    int pos = buffer.getLength();
    buffer.writeInt(0); // write 4 bytes for future length
    buffer.writeInt(request.getTag());
    buffer.writeInt(request.getSeqNum());
    request.encode(buffer);
    buffer.replaceInt(pos, buffer.getLength() - pos - 4);
    buffer.writeData(os);
    os.flush();
  }

  private <T extends PDU> T sendPDU(PDU request, T... expectedResponsesInst) throws IOException {
    request.assignSeqNum();
    serialize(request, os);

    BufferReader buffer = new BufferReader(1024);
    buffer.fill(is, 4);
    int len = buffer.readInt();
    if (len > 0)
      buffer.fill(is, len);

    int tag = buffer.readInt();
    int seqNum = buffer.readInt();

    if (seqNum != request.getSeqNum())
      throw new IOException("Unexpected response sequence number: " + seqNum + ". Expected value: " + request.getSeqNum());

    if (expectedResponsesInst == null || expectedResponsesInst.length == 0)
      return null;

    for (T resp : expectedResponsesInst) {
      if (resp.getTag() == tag) {
        resp.decode(buffer);
        return resp;
      }
    }

    StringBuilder expectedTags = new StringBuilder();
    for (T resp : expectedResponsesInst)
      expectedTags.append(resp.getTag()).append(' ');
    throw new IOException("Unexpected response tag: " + tag + ". Expected tags: " + expectedTags.toString());
  }

  protected PDU request(PDU request, PDU... expectedResponsesInst) throws IOException {

    try {
      sendLock.lock();

      if (logger.isDebugEnabled())
        logger.debug("Sending request: " + request);

      if (socket == null)
        reconnect();

      PDU resp;
      // При первой попытке отправки допустима ошибка
      try {
        resp = sendPDU(request, expectedResponsesInst);
      } catch (Exception ignored) {
        // Если отправить не удалось, реконнектимся и снова отправляем
        reconnect();
        resp = sendPDU(request, expectedResponsesInst);
      }
      
      if (logger.isDebugEnabled()) {
        logger.debug("Request sent: " + request);
        logger.debug("Response received:" + resp);
      }

      return resp;

    } finally {
      sendLock.unlock();
    }
  }

  protected abstract void onConnect() throws IOException;

  private void reconnect() throws IOException {
    try {
      close();

      logger.debug("Connecting to " + host + ':' + port + " ...");

      socket = new Socket(host, port);
      socket.setSoTimeout(timeout);
      is = socket.getInputStream();
      os = socket.getOutputStream();

      logger.debug("Connected to " + host + ':' + port + " .");

    } catch (IOException e) {
      logger.error("Could not connect to " + host + ':' + port + ".", e);
      throw new ServerOfflineException();
    }

    onConnect();
  }

  public void close() {
    if (is != null)
      try {
        is.close();
      } catch (IOException ignored) {
      }
    if (os != null)
      try {
        os.close();
      } catch (IOException ignored) {
      }
    if (socket != null)
      try {
        socket.close();
      } catch (IOException ignored) {
      }
  }
}
