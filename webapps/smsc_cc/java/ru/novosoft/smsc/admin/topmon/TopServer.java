package ru.novosoft.smsc.admin.topmon;

import ru.novosoft.smsc.util.InetAddress;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.util.LinkedList;
import java.util.List;


public class TopServer extends Thread {
  private final org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(this.getClass());
  private final int appletPort;
  private String smscHost = null;
  private final int topPort;
  private ServerSocket ssock;
  private final List<TopServerHandler> handlers = new LinkedList<TopServerHandler>();

  public TopServer(int appletPort, InetAddress smsc) throws IOException {
    this.appletPort = appletPort;
    smscHost = smsc != null ? smsc.getHost() : null;
    topPort = smsc != null ? smsc.getPort() : 0;
    ssock = new ServerSocket(appletPort);
  }

  public String getSmscHost() {
    return smscHost;
  }

  public int getTopPort() {
    return topPort;
  }

  void addRunner(TopServerHandler handler) {
    synchronized (handlers) {
      handlers.add(handler);
    }
  }

  public void removeRunner(TopServerHandler handler) {
    synchronized (handlers) {
      handlers.remove(handler);
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

  protected TopServerHandler createClientHandler(Socket sock) throws IOException {
    return new TopServerHandler(sock, this);
  }

  public void run() {
    try {
      while (!isStopping) {
        Socket sock;
        try {
          sock = ssock.accept();
          try {
            logger.debug("TopMon: Client " + sock.getInetAddress().getHostAddress() + " connected");
            TopServerHandler sr = createClientHandler(sock);
            addRunner(sr);
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
        } catch (IOException ex) {
          logger.error("Error accepting connection", ex);
          break;
        }
      }
      synchronized (shutSemaphore) {
        shutSemaphore.notifyAll();
      }
    } catch (Throwable ex) {
      logger.error("Unexpected exception occured", ex);
    }
  }

}
