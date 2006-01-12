/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.scmon;

import ru.sibinco.scag.perfmon.PerfServerRunner;
import ru.sibinco.lib.backend.util.config.Config;

import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.util.ArrayList;
import java.util.Properties;
import java.io.IOException;

public class ScServer extends Thread {
    private org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(this.getClass());
    protected int appletPort;
    protected String smscHost;
    protected int topPort;
    ServerSocket ssock;
    ArrayList runners;

    public ScServer(Config config) throws IOException, Config.ParamNotFoundException, Config.WrongParamTypeException {
        appletPort = (int) config.getInt("scmon.appletport");
        smscHost = config.getString("scmon.schost");
        topPort = (int) config.getInt("scmon.scport");
        ssock = new ServerSocket(appletPort);
        runners = new ArrayList();
    }

    public ScServer(Properties config) throws IOException, Config.ParamNotFoundException, Config.WrongParamTypeException {
        appletPort = getConfigInt(config, "scmon.appletport");
        smscHost = config.getProperty("smsc.host");
        topPort = getConfigInt(config, "scmon.smscport");
        ssock = new ServerSocket(appletPort);
        runners = new ArrayList();
    }

    int getConfigInt(Properties config, String key) {
        return Integer.valueOf(config.getProperty(key)).intValue();
    }

    public int getAppletPort() {
        return appletPort;
    }

    public String getSmscHost() {
        return smscHost;
    }

    public int getTopPort() {
        return topPort;
    }

    public void addRunner(ScServerRunner runner) {
        synchronized (runners) {
            runners.add(runner);
        }
    }

    public void removeRunner(ScServerRunner runner) {
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
                        logger.debug("ScMon: Client " + sock.getInetAddress().getHostAddress() + " connected");
                        ScServerRunner sr = new ScServerRunner(sock, this);
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

//    public static void main(String args[]) {
//        Properties config = new Properties();
//        try {
//            config.load(new FileInputStream(args[0]));
//            ScServer topServer = new ScServer(config);
//            topServer.start();
//        } catch (Exception e) {
//            e.printStackTrace();
//        }
//    }

}

