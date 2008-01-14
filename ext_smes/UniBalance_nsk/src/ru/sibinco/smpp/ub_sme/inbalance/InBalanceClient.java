package ru.sibinco.smpp.ub_sme.inbalance;

import org.apache.log4j.Category;
import ru.sibinco.smpp.ub_sme.InitializationException;

import java.io.EOFException;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;


/**
 * Copyright (c)
 * EyeLine Communications
 * All rights reserved.
 */
public class InBalanceClient {
  private final static Category logger = Category.getInstance(InBalanceClient.class);

  private Socket socket = null;
  private InputStream is;
  private OutputStream os;

  private String serverHost;
  private int serverPort;

  private boolean connected = false;
  private boolean shutdown = false;

  private Object writeMonitor = new Object();
  private Object readMonitor = new Object();
  private Object connectMonitor = new Object();

  private InBalancePDUHandler pduHandler;

  private int nextSequenceNumber = 0;

  public InBalanceClient(String serverHost, String serverPort, InBalancePDUHandler pduHandler) throws InitializationException {
    if (serverHost == null) {
      throw new InitializationException("Server host is null");
    }
    if (serverPort == null) {
      throw new InitializationException("Server port is null");
    }
    this.serverHost = serverHost;
    try {
      this.serverPort = Integer.parseInt(serverPort);
    } catch (NumberFormatException e) {
      throw new InitializationException("Bad server port value: " + serverPort);
    }
    this.pduHandler = pduHandler;
    (new IncomingPDUController()).start();
  }

  public boolean connect() {
    synchronized (connectMonitor) {
      if (isConnected()) {
        return true;
      }
      if (shutdown) {
        return false;
      }
      if (logger.isDebugEnabled()) logger.debug("Connecting to server");
      try {
        socket = new Socket(serverHost, serverPort);
        socket.setKeepAlive(true);
        is = socket.getInputStream();
        os = socket.getOutputStream();
      } catch (IOException e) {
        logger.error("I/O Error: " + e, e);
        connected = false;
        return false;
      }
      connected = true;
      if (logger.isInfoEnabled()) logger.info("Connected to " + serverHost + ":" + serverPort);
      connectMonitor.notifyAll();
    }
    return true;
  }

  protected void disconnect() {
    synchronized (writeMonitor) {
      if (logger.isDebugEnabled()) logger.debug("Disconnecting from server");
      connected = false;
      try {
        if (socket != null) {
          socket.close();
          if (logger.isInfoEnabled()) logger.info("Disconnected");
        }
      } catch (IOException e) {
        logger.warn("I/O Error at socket close: " + e, e);
      }
      connected = false;
      socket = null;
      is = null;
      os = null;
    }
  }

  public boolean isConnected() {
    return (socket != null && (socket != null && !socket.isClosed() && socket.isConnected()) && connected);
  }

  protected byte[] readPDU() throws IOException {
    if (!isConnected()) {
      if (!connect()) {
        throw new IOException("Couldn't connect to server");
      }
    }
    byte pdu[];
    int[] len_buf = new int[4];
    try {
      synchronized (readMonitor) {
        int pduLength = readLength(len_buf);
        pdu = new byte[pduLength + 4];
        pdu[0] = (byte) len_buf[0];
        pdu[1] = (byte) len_buf[1];
        pdu[2] = (byte) len_buf[2];
        pdu[3] = (byte) len_buf[3];
        readBody(4, pduLength, pdu);
      }
      if (logger.isDebugEnabled())
        logger.debug("Receive PDU: " + InBalancePDU.bytesDebugString(pdu));

    } catch (IOException e) {
      disconnect();
      throw e;
    }
    return pdu;
  }

  private int readLength(int[] buf) throws IOException {
    buf[0] = is.read();
    buf[1] = is.read();
    buf[2] = is.read();
    buf[3] = is.read();
    if (buf[0] == -1 || buf[1] == -1 || buf[2] == -1 || buf[3] == -1) throw new EOFException();
    return ((buf[0] & 0xFF) << 24) | ((buf[1] & 0xFF) << 16) | ((buf[2] & 0xFF) << 8) | buf[3] & 0xFF;
  }

  private void readBody(int pos, int size, byte[] data) throws IOException {
    int read = 0;
    while (size > 0 && (read = is.read(data, pos, size)) != -1) {
      pos += read;
      size -= read;
    }
  }

  protected void writePDU(InBalancePDU pdu) throws IOException, InBalancePDUException {
    if (!isConnected()) {
      if (!connect()) {
        throw new IOException("Couldn't connect to server");
      }
    }


    if (logger.isDebugEnabled())
      logger.debug("Send InBalancePDU: " + pdu);

    byte[] data = pdu.getData();

    if (logger.isDebugEnabled())
      logger.debug(InBalancePDU.bytesDebugString(data));

    try {
      synchronized (writeMonitor) {
        os.write(data);
        os.flush();
      }
    } catch (IOException e) {
      disconnect();
      throw e;
    }
  }

  public void close() {
    shutdown = true;
    disconnect();
  }

  public synchronized int assignRequestId() {
    return nextSequenceNumber++;
  }

  public void sendBalanceRequest(String MsISDN, String UssData, byte IN_SSN, String IN_ISDN, int requestId) throws InBalanceClientException {
    InBalanceRequest request = new InBalanceRequest();
    request.setRequestId(requestId);
    request.setUssData(UssData);
    request.setMsISDN(MsISDN);
    request.setIN_SSN(IN_SSN);
    request.setIN_ISDN(IN_ISDN);
    try {
      writePDU(request);
    } catch (Exception e) {
      throw new InBalanceClientException(e);
    }
  }

  class IncomingPDUController extends Thread {
    public void run() {
      while (!shutdown) {
        if (isConnected()) {
          try {
            byte[] pduData = readPDU();
            try {
              InBalanceResult pdu = new InBalanceResult(pduData);
              pduHandler.handleInBalancePDU(pdu);
            } catch (InBalancePDUException e) {
              logger.error("PDU parse error", e);
            }
          } catch (IOException e) {
            logger.error("IO Error", e);
          }
        } else {
          synchronized (connectMonitor) {
            try {
              connectMonitor.wait();
            } catch (InterruptedException e) {
              logger.warn("IncomingPDUController was interrupted: " + e, e);
            }
          }
        }
      }
    }
  }

}
