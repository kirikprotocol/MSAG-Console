package ru.novosoft.smsc.admin.perfmon;


import ru.novosoft.smsc.util.InetAddress;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.util.LinkedList;
import java.util.List;


public class PerfServer extends Thread {
  private final org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(this.getClass());
  private final int appletPort;
  private final InetAddress address;
  private final ServerSocket ssock;
  private final List<MonitorClientHandler> handlers = new LinkedList<MonitorClientHandler>();
  private final boolean support64Bit;

  public InetAddress getSmscAddress() {
    return new InetAddress(address);
  }

  public PerfServer(int appletPort, InetAddress address, boolean support64Bit) throws IOException {
    this.appletPort = appletPort;
    this.address = address;
    this.support64Bit = support64Bit;
    ssock = new ServerSocket(appletPort);
  }

  public void addHandler(MonitorClientHandler runner) {
    synchronized (handlers) {
      handlers.add(runner);
    }
  }

  public void removeHandler(MonitorClientHandler runner) {
    synchronized (handlers) {
      handlers.remove(runner);
    }
  }

  private boolean isStopping = false;
  private final Object shutSemaphore = new Object();

  public void shutdown() {
    logger.debug("shutdown called");
    synchronized (shutSemaphore) {
      isStopping = true;
      try {
        ssock.close();
      } catch (Exception e) {
        logger.error("Can't close server socket", e);
      }
      try {
        shutSemaphore.wait();
      } catch (InterruptedException ignored) {
      }
    }
    while (handlers.size() > 0) {
      handlers.get(0).shutdown();
    }
  }

  protected void handleClientSocket(Socket sock) throws IOException {
    try {
      logger.debug("Client " + sock.getInetAddress().getHostAddress() + " connected");
      MonitorClientHandler sr = new MonitorClientHandler(sock, this, support64Bit);
      addHandler(sr);
      sr.start();
    } catch (SocketException ee) {
      logger.warn("User connection socket error " + ee.getMessage());
      try {
        sock.close();
      } catch (IOException ex) {
        logger.warn("Can't close client socket", ex);
      }
    } catch (IOException ee) {
      logger.warn("User connection error", ee);
      try {
        sock.close();
      } catch (IOException ex) {
        logger.warn("Can't close client socket", ex);
      }
    }
  }

  public void run() {
    try {
      while (!isStopping) {
        Socket sock;
        try {
          sock = ssock.accept();
          handleClientSocket(sock);
        } catch (IOException ex) {
          logger.error("Error accepting connection", ex);
          break;
        }
      }
      synchronized (shutSemaphore) {
        shutSemaphore.notifyAll();
      }
    } catch (Exception ex) {
      logger.error("Unexpected exception occured", ex);
    }
  }
}
