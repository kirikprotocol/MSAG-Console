package mobi.eyeline.informer.admin.protogen;

import mobi.eyeline.protogen.framework.BufferReader;
import mobi.eyeline.protogen.framework.BufferWriter;
import mobi.eyeline.protogen.framework.PDU;
import org.apache.log4j.Logger;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketTimeoutException;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * @author Artem Snopkov
 */
public abstract class SyncProtogenConnection {

  private static final Logger logger = Logger.getLogger(SyncProtogenConnection.class);
  private static final int DEFAULT_CONNECT_TIMEOUT = 5000;

  private final Lock sendLock = new ReentrantLock();

  private final String host;
  private final int port;
  private final int timeout;
  private final int connectTimeout;

  private Socket socket;
  private InputStream is;
  private OutputStream os;

  protected SyncProtogenConnection(String host, int port, int timeout, int connectTimeout) {
    this.host = host;
    this.port = port;
    this.timeout = timeout;
    this.connectTimeout = connectTimeout;
  }

  protected SyncProtogenConnection(String host, int port, int timeout) {
    this(host, port, timeout, DEFAULT_CONNECT_TIMEOUT);
  }

  public String getHost() {
    return host;
  }

  public int getPort() {
    return port;
  }

  private static void serialize(PDU request, OutputStream os) throws IOException {
    BufferWriter writer = new BufferWriter();
    int pos = writer.size();
    writer.appendInt(0); // write 4 bytes for future length
    writer.appendInt(request.getTag());
    writer.appendInt(request.getSeqNum());
    request.encode(writer);
    int len = writer.size()-pos-4;
    writer.replaceInt( len,  pos); // fill first 4 bytes with actual length

    if (logger.isDebugEnabled())
      logger.debug("Sending PDU: " + writer.getHexDump());

    writer.writeBuffer(os);
    os.flush();
  }

  private <T extends PDU> T sendPDU(PDU request, T... expectedResponsesInst) throws IOException {
    request.assignSeqNum();
    serialize(request, os);

    BufferReader buffer = new BufferReader(1024);
    buffer.fillFully(is, 4);
    int len = buffer.removeInt();

    if (logger.isDebugEnabled())
      logger.debug("Received packet len=" + len);
    if (len > 0)
      buffer.fillFully(is, len);

    if (logger.isDebugEnabled())
      logger.debug("PDU received: " + buffer.getHexDump());

    int tag = buffer.removeInt();
    int seqNum = buffer.removeInt();

    if (seqNum != request.getSeqNum())
      throw new IOException("Unexpected response sequence number: " + seqNum + ". Expected value: " + request.getSeqNum());

    if (expectedResponsesInst == null || expectedResponsesInst.length == 0)
      return null;

    for (T resp : expectedResponsesInst) {
      if (resp.getTag() == tag) {
        resp.setSeqNum(seqNum);
        resp.decode(buffer);
        return resp;
      }
    }

    StringBuilder expectedTags = new StringBuilder();
    for (T resp : expectedResponsesInst)
      expectedTags.append(resp.getTag()).append(' ');
    throw new IOException("Unexpected response tag: " + tag + ". Expected tags: " + expectedTags.toString());
  }

  protected <T extends PDU> T request(PDU request, T... expectedResponsesInst) throws IOException {

    try {
      sendLock.lock();

      if (logger.isDebugEnabled())
        logger.debug("Sending request: " + request);

      if (socket == null)
        reconnect();

      T resp;
      // При первой попытке отправки допустима ошибка
      try {
        resp = sendPDU(request, expectedResponsesInst);
      } catch (SocketTimeoutException e) {
        logger.error("Response wait timeout reached for seq=" + request.getSeqNum() + ".");
        throw new ResponseWaitTimeoutException();
      } catch (IOException ignored) {
        logger.error("Connection lost. Cause: " + ignored.getClass() + " : "+ ignored.getMessage() + ". Try to reconnect.");
        // Если отправить не удалось, реконнектимся и снова отправляем
        reconnect();
        resp = sendPDU(request, expectedResponsesInst);
      } catch (Exception e) {
        logger.error(e,e);
        throw new IOException(e.getMessage());
      }
      
      if (logger.isDebugEnabled()) {
        logger.debug("Response received:" + resp);
      }

      return resp;
    } catch (IOException e) {
      close();
      throw e;
    } finally {
      sendLock.unlock();
    }
  }

  protected abstract void onConnect() throws IOException;

  private void reconnect() throws IOException {
    try {
      close();

      if (logger.isDebugEnabled())
        logger.debug("Connecting to " + host + ':' + port + " ...");

      socket = new Socket();
      socket.connect(new InetSocketAddress(host, port), connectTimeout);
      socket.setSoTimeout(timeout);
      is = socket.getInputStream();
      os = socket.getOutputStream();

      if (logger.isDebugEnabled())
        logger.debug("Connected to " + host + ':' + port + " .");

    } catch (IOException e) {
      logger.error("Could not connect to " + host + ':' + port + ".", e);

      if (socket != null) {
        try {
          socket.close();
        } catch (IOException ignored) {}
        socket = null;
      }
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
