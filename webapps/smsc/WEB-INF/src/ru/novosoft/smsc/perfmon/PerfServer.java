package ru.novosoft.smsc.perfmon;

import ru.novosoft.smsc.util.config.Config;

import java.net.*;
import java.io.IOException;
import java.util.ArrayList;

import org.apache.log4j.Category;

public class PerfServer extends Thread {
    private org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(this.getClass());
    protected int appletPort;
    protected String smscHost;
    protected int perfPort;
    ServerSocket ssock;
    ArrayList runners;

    public String getSmscHost() {
        return smscHost;
    }

    public int getPerfPort() {
        return perfPort;
    }


    public PerfServer(Config config) throws IOException, Config.ParamNotFoundException, Config.WrongParamTypeException {
        appletPort = config.getInt("perfmon.appletport");
        smscHost = config.getString("smsc.host");
        perfPort = config.getInt("perfmon.smscport");
        ssock = new ServerSocket(appletPort);
        runners = new ArrayList();
    }

    public void addRunner(PerfServerRunner runner) {
        synchronized (runners) {
            runners.add(runner);
        }
    }

    public void removeRunner(PerfServerRunner runner) {
        synchronized (runners) {
            runners.remove(runner);
        }
    }

    boolean isStopping = false;
    Object shutSemaphore = new Object();

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
            } catch (InterruptedException e) {
            }
        }
        while (runners.size() > 0) {
            ((PerfServerRunner) runners.get(0)).shutdown();
        }
    }

    public void run() {
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
                } catch (IOException ee) {
                    logger.warn("User connection error", ee);
                    try {
                      sock.close();
                    } catch (IOException ex) {
                      logger.warn("Can't close client socket", ee);
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
