package ru.sibinco.smpp.ub_sme;

import java.io.File;

class ResponsePatternConfigControllerThread extends Thread {
    private final static org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(ResponsePatternConfigControllerThread.class);
    private boolean started = false;
    private Object monitor = new Object();
    private Object shutmonitor = new Object();

    private long pollingInterval = 60000L;
    private long fileLastModified;
    private File responsePatternConfigFile;


    public ResponsePatternConfigControllerThread(long pollingInterval, File responsePatternConfigFile) {
        this.pollingInterval = pollingInterval;
        this.responsePatternConfigFile = responsePatternConfigFile;
        this.fileLastModified = responsePatternConfigFile.lastModified();
    }

    public void startService() {
        started = true;
        if (logger.isInfoEnabled()) logger.info(getName() + " started.");
        start();
    }

    public void stopService() {
        synchronized (shutmonitor) {
            synchronized (monitor) {
                started = false;
                monitor.notifyAll();
            }
            try {
                shutmonitor.wait();
            } catch (InterruptedException e) {
                logger.warn(getName() + " shutting down exception:", e);
            }
        }
    }

    public void run() {
        while (started) {
            if (logger.isDebugEnabled())
                logger.debug("Check response pattern config file modification date");
            if (responsePatternConfigFile.lastModified() != fileLastModified) {
                if (logger.isInfoEnabled())
                    logger.info("Response pattern config file was modificated. Reloading response pattern config.");
                try {
                    ResponsePatternManager.getResponsePatternManager().loadPatterns(responsePatternConfigFile);
                    fileLastModified = responsePatternConfigFile.lastModified();
                } catch (InitializationException e) {
                    logger.error("Error while reloading response pattern config: " + e, e);
                }
            }

            synchronized (monitor) {
                if (!started) break;
                try {
                    monitor.wait(pollingInterval);
                } catch (InterruptedException e) {
                    logger.warn(getName() + " was interrupted.", e);
                }
            }
        }
        synchronized (shutmonitor) {
            shutmonitor.notifyAll();
        }

    }
}

