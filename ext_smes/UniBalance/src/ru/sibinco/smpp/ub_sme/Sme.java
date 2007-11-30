package ru.sibinco.smpp.ub_sme;

import ru.aurorisoft.smpp.Multiplexor;
import ru.aurorisoft.smpp.SMPPException;
import ru.sibinco.smpp.ub_sme.util.DBConnectionManager;
import ru.sibinco.smpp.ub_sme.util.Utils;
import ru.sibinco.smpp.ub_sme.InitializationException;
import ru.sibinco.util.threads.ThreadsPool;

import java.io.IOException;
import java.net.URL;
import java.util.Properties;
import java.util.jar.JarInputStream;
import java.util.jar.Manifest;

public class Sme {

    private final static org.apache.log4j.Category Logger = org.apache.log4j.Category.getInstance(Sme.class);

    public static String version = " SibInfoConsulting UniBalance ";
    private static SmeEngine smeEngine;

    static {
        URL codeBase = Sme.class.getProtectionDomain().getCodeSource().getLocation();
        if (codeBase.getPath().endsWith("UniBalance.jar")) {
            JarInputStream jin = null;
            try {
                jin = new JarInputStream(codeBase.openStream());
                Manifest mf = jin.getManifest();
                version = mf.getMainAttributes().getValue("Customer") + version;
                version = version + mf.getMainAttributes().getValue("Version");
            } catch (Exception e) {
                e.printStackTrace();
            } finally {
                if (jin != null)
                    try {
                        jin.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
            }
        }
    }

    public Sme(String globalConfig) throws InitializationException {

        // load global config
        Properties config;
        try {
            config = new Properties();
            config.load(this.getClass().getClassLoader().getResourceAsStream(globalConfig));
        } catch (IOException e) {
            Logger.error("Exception occured during loading global configuration.", e);
            throw new InitializationException("Exception occured during loading global configuration.", e);
        }

        // init DB connection manager
        /*
        try {
            DBConnectionManager.getInstance();
        } catch (InitializationException e) {
            Logger.error("Could not get DB connection manager.", e);
            throw new InitializationException("Could not get DB connection manager.", e);
        }
          */
        boolean outqueueEnabled = Utils.getBooleanProperty(config, "outqueue.enabled", true);

        OutgoingQueue outQueue = null;
        if (outqueueEnabled) {
            // init ougoing queue
            long deliveryTimeout = 0;
            try {
                deliveryTimeout = Utils.getLongProperty(config, "delivery.timeout", 30000L);
            } catch (NullPointerException e) {
                Logger.warn("\"delivery.timeout\" property not specified.");
            } catch (NumberFormatException e) {
                Logger.warn("\"delivery.timeout\" property is invalid: " + config.getProperty("delivery.timeout"));
            }
            long retryPeriod = 0;
            try {
                retryPeriod = Utils.getLongProperty(config, "retry.period", 60000L);
            } catch (NullPointerException e) {
                Logger.warn("\"retry.period\" property not specified.");
            } catch (NumberFormatException e) {
                Logger.warn("\"retry.period\" property is invalid: " + config.getProperty("retry.period"));
            }
            int maxRetries = 0;
            try {
                maxRetries = Utils.getIntProperty(config, "max.retries", 1);
            } catch (NullPointerException e) {
                Logger.warn("\"max.retries\" property not specified.");
            } catch (NumberFormatException e) {
                Logger.warn("\"max.retries\" property is invalid: " + config.getProperty("max.retries"));
            }

            outQueue = new OutgoingQueue(deliveryTimeout, maxRetries, retryPeriod);

            // start outgoing queue controller
            long outqueueControllerPollinginterval = 0;
            try {
                outqueueControllerPollinginterval = Long.parseLong(config.getProperty("outqueue.controller.pollinginterval"));
            } catch (NullPointerException e) {
                Logger.warn("\"outqueue.controller.pollinginterval\" property not specified.");
            } catch (NumberFormatException e) {
                Logger.warn("\"outqueue.controller.pollinginterval\" property is invalid: " + config.getProperty("outqueue.controller.pollinginterval"));
            }
            new OutgoingQueueController(outQueue, "OutgoingQueueController", outqueueControllerPollinginterval).startService();
        }

        // init multiplexor
        Multiplexor multiplexor = new Multiplexor();
        try {
            multiplexor.init(config, "", null);
            if (Logger.isInfoEnabled()) Logger.info("Network multiplexor initialized.");
        } catch (SMPPException e) {
            Logger.error("Exception occured during initializing of multiplexor.", e);
            throw new InitializationException("Exception occured during initializing of multiplexor.");
        }

        MessageSender messageSender = null;
        if (outqueueEnabled) {
            //init abonentRequest sender
            messageSender = new MessageSender(outQueue, multiplexor, "MessageSender", Utils.getLongProperty(config, "abonentRequest.sender.pollinginterval", 10000L));
            messageSender.setSyncSendTimeout(Utils.getLongProperty(config, "sync.mode.send.timeout", 10000L));
            messageSender.setSendDelay(Utils.getLongProperty(config, "send.delay", 0));

            outQueue.setMessageSender(messageSender);
        }

        // connect multiplexor
        try {
            multiplexor.connect();
        } catch (SMPPException e) {
            Logger.fatal("Exception occured during establishing connection in multiplexor.", e);
            throw new InitializationException("Exception occured during establishing connection in multiplexor.");
        }

        if (outqueueEnabled) {
            // run abonentRequest sender
            messageSender.startService();
        }

        // Init threads pool
        ThreadsPool threadsPool = new ThreadsPool(config, "threads.pool");

        // SME engine
        //TODO переделать
        smeEngine = new SmeEngine(multiplexor, outQueue, threadsPool);
        smeEngine.init(config);
        ProfileAbonentManager.init(config);

        multiplexor.setMessageListener(smeEngine);
        multiplexor.setResponseListener(smeEngine);

    }

    public static void main(String[] args) {
        if (args.length != 1) {
            System.err.println("Use: java ru.sibinco.smpp.ub_sme.Sme <global_config_file>");
        } else {
            try {
                new Sme(args[0]);
            } catch (InitializationException e) {
                System.err.println("Exception occured during runnig Sme: " + e.getMessage() + " See logs for details.");
                e.printStackTrace(System.err);
            }
        }
    }

    public static SmeEngine getSmeEngine() {
        return smeEngine;
    }
}
