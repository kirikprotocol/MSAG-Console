package ru.novosoft.smsc.topmon;

import ru.novosoft.smsc.perfmon.PerfServerRunner;
import ru.novosoft.smsc.util.config.Config;

import java.io.FileInputStream;
import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.util.ArrayList;
import java.util.Properties;

/**
 * Created by: Serge Lugovoy
 * Date: 14.05.2004
 * Time: 12:14:26
 */
public class TopServer extends Thread
{
  private org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(this.getClass());
  protected int appletPort;
  protected String smscHost = null;
  protected int topPort;
  ServerSocket ssock;
  ArrayList runners;

  public TopServer(Config config) throws IOException, Config.ParamNotFoundException, Config.WrongParamTypeException
  {
    appletPort = config.getInt("topmon.appletport");
    topPort = config.getInt("topmon.smscport");
    ssock = new ServerSocket(appletPort);
    runners = new ArrayList();
  }

  public TopServer(Properties config) throws IOException, Config.ParamNotFoundException, Config.WrongParamTypeException
  {
    appletPort = getConfigInt(config, "topmon.appletport");
    smscHost = config.getProperty("smsc.host");
    topPort = getConfigInt(config, "topmon.smscport");
    ssock = new ServerSocket(appletPort);
    runners = new ArrayList();
  }

  int getConfigInt(Properties config, String key)
  {
    return Integer.valueOf(config.getProperty(key)).intValue();
  }

  public int getAppletPort()
  {
    return appletPort;
  }

  public String getSmscHost()
  {
    return smscHost;
  }

	public void setSmscHost(String smscHost)
	{
		this.smscHost = smscHost;
	}

  public int getTopPort()
  {
    return topPort;
  }

  public void addRunner(TopServerRunner runner)
  {
    synchronized (runners) {
      runners.add(runner);
    }
  }

  public void removeRunner(TopServerRunner runner)
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
            logger.debug("TopMon: Client " + sock.getInetAddress().getHostAddress() + " connected");
            TopServerRunner sr = new TopServerRunner(sock, this);
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

  public static void main(String args[])
  {
    Properties config = new Properties();
    try {
      config.load(new FileInputStream(args[0]));
      TopServer topServer = new TopServer(config);
      topServer.start();
    } catch (Exception e) {
      e.printStackTrace();
    }
  }

}
