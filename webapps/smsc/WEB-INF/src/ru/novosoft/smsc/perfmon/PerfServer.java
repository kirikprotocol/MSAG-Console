package ru.novosoft.smsc.perfmon;

import ru.novosoft.smsc.util.config.Config;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.util.ArrayList;

public class PerfServer extends Thread
{
  private org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(this.getClass());
  protected int appletPort;
  protected String smscHost = null;
  protected int perfPort;
  ServerSocket ssock;
  ArrayList runners;

  public String getSmscHost()
  {
    return smscHost;
  }

	public void setSmscHost(String smscHost)
	{
		this.smscHost = smscHost;
	}

  public int getPerfPort()
  {
    return perfPort;
  }


  public PerfServer(Config config) throws IOException, Config.ParamNotFoundException, Config.WrongParamTypeException
  {
    appletPort = config.getInt("perfmon.appletport");
    perfPort = config.getInt("perfmon.smscport");
    ssock = new ServerSocket(appletPort);
    runners = new ArrayList();
  }

  public void addRunner(PerfServerRunner runner)
  {
    synchronized (runners) {
      runners.add(runner);
    }
  }

  public void removeRunner(PerfServerRunner runner)
  {
    synchronized (runners) {
      runners.remove(runner);
    }
  }

  boolean isStopping = false;
  Object shutSemaphore = new Object();

  public void shutdown()
  {
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
      } catch (InterruptedException e) {
      }
    }
    while (runners.size() > 0) {
      ((PerfServerRunner) runners.get(0)).shutdown();
    }
  }

  public void run()
  {
    try {
      while (!isStopping) {
        Socket sock = null;
        try {
          sock = ssock.accept();
          try {
            logger.debug("Client " + sock.getInetAddress().getHostAddress() + " connected");
            PerfServerRunner sr = new PerfServerRunner(sock, this);
            addRunner(sr);
            sr.start();
          } catch (SocketException ee) {
            logger.warn("User connection socket error " + ee.getMessage());
            if (sock != null) {
              try {
                sock.close();
              } catch (IOException ex) {
                logger.warn("Can't close client socket", ex);
              }
            }
          } catch (IOException ee) {
            logger.warn("User connection error", ee);
            if (sock != null) {
              try {
                sock.close();
              } catch (IOException ex) {
                logger.warn("Can't close client socket", ex);
              }
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
