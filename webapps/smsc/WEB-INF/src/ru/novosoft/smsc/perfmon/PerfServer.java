package ru.novosoft.smsc.perfmon;

import ru.novosoft.smsc.util.config.Config;
import java.net.*;
import java.io.IOException;
import java.util.ArrayList;

import org.apache.log4j.Category;

public class PerfServer extends Thread {
    private org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(this.getClass());
    protected int    appletPort;
    protected String smscHost;
    protected int    perfPort;
    ServerSocket     ssock;
    ArrayList        runners;

    public PerfServer( Config config ) throws IOException, Config.ParamNotFoundException, Config.WrongParamTypeException
    {
        appletPort = config.getInt("perfmon.appletport");
        smscHost = config.getString("smsc.host");
        perfPort = config.getInt("perfmon.smscport");
        ssock = new ServerSocket( appletPort );
        runners = new ArrayList();
    }

    public void addRunner( PerfServerRunner runner ) {
        synchronized (runners) {
            runners.add(runner);
        }
    }

    public void removeRunner( PerfServerRunner runner ) {
        synchronized (runners) {
            runners.remove(runner);
        }
   }

    boolean isStopping = false;
    Object shutSemaphore = new Object();

    public void shutdown() {
        logger.debug( "shutdown called");
        synchronized( shutSemaphore ) {
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
        while( runners.size() > 0 ) {
            ((PerfServerRunner)runners.get(0)).shutdown();
        }
    }

    public void run() {
        while(!isStopping) {
            Socket sock = null;
            try {
                sock = ssock.accept();
                logger.debug("Client "+sock.getInetAddress().getHostAddress()+" connected");
                PerfServerRunner sr = new PerfServerRunner( sock, this );
                addRunner(sr);
                sr.start();
            } catch (IOException ex) {
                logger.warn("Error accepting connection", ex );
            }
        }
        synchronized( shutSemaphore ) {
            shutSemaphore.notifyAll();
        }
    }
}
