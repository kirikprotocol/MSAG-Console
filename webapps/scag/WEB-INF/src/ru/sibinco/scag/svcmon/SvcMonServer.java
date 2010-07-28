/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.svcmon;

import org.apache.log4j.Category;

import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.util.ArrayList;
import java.util.Properties;
import java.io.IOException;
import java.io.FileInputStream;
import java.io.FileNotFoundException;

import ru.sibinco.lib.backend.util.config.Config;

/**
 * The <code>SvcMonServer</code> class represents
 * <p><p/>
 * Date: 07.12.2005
 * Time: 14:46:36
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class SvcMonServer extends Thread {

    private Category logger = Category.getInstance(SvcMonServer.class);
    protected int appletPort;
    protected String svcHost;
    protected int svcPort;

    ServerSocket serverSocket;
    ArrayList runners;

    public SvcMonServer(Config config) throws IOException, Config.ParamNotFoundException, Config.WrongParamTypeException {

        appletPort = (int) config.getInt("svcmon.appletport");
        svcHost = config.getString("svcmon.svcHost");
        svcPort = (int) config.getInt("svcmon.svcPort");
        serverSocket = new ServerSocket(appletPort);
        runners = new ArrayList();
    }

    public SvcMonServer(Properties config) throws IOException, Config.ParamNotFoundException, Config.WrongParamTypeException {
        appletPort = getConfigInt(config, "svcmon.appletport");
        svcHost = config.getProperty("svcmon.svcHost");
        svcPort = (int) getConfigInt(config, "svcmon.svcPort");
        serverSocket = new ServerSocket(appletPort);
        runners = new ArrayList();

    }

    int getConfigInt(Properties config, String key) {
        return Integer.valueOf(config.getProperty(key)).intValue();
    }

    public int getAppletPort() {
        return appletPort;
    }

    public String getSvcHost() {
        return svcHost;
    }

    public int getSvcPort() {
        return svcPort;
    }

    public void addRunner(SvcMonServerRunner runner) {
        synchronized (runners) {
            runners.add(runner);
        }
    }

    public void removeRunner(SvcMonServerRunner runner) {
        synchronized (runners) {
            runners.remove(runner);
        }
    }

    boolean isStopping = false;
    Object shutSemaphore = new Object();

    public void shutdown() {
        logger.debug("shutdown called ");
        synchronized (shutSemaphore) {
            isStopping = true;
            try {
                serverSocket.close();
            } catch (IOException e) {
                logger.error("Can't close server socket ", e);
            }
            try {
                shutSemaphore.wait();
            } catch (InterruptedException e) {
            }
        }
        while (runners.size() > 0) {
            ((SvcMonServerRunner) runners.get(0)).shutdown();
        }
    }

    public void run() {
        try {
            while (!isStopping) {
                Socket sock = null;
                try {
                    sock = serverSocket.accept();
                    //System.out.println("SvcMon: Client " + sock.getInetAddress().getHostAddress() + " connected");
                    try {
                        logger.debug("SvcMon: Client " + sock.getInetAddress().getHostAddress() + " connected");
                        SvcMonServerRunner sr = new SvcMonServerRunner(sock, this);
                        addRunner(sr);
                        sr.start();
                    } catch (SocketException ee) {
                        ee.printStackTrace();
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
