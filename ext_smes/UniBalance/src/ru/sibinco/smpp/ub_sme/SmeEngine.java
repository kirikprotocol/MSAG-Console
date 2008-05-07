package ru.sibinco.smpp.ub_sme;

import com.logica.smpp.Data;
import ru.aurorisoft.smpp.*;
import ru.sibinco.smpp.ub_sme.mg.MGRequestManager;
import ru.sibinco.util.threads.ThreadsPool;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.text.DecimalFormatSymbols;
import java.text.MessageFormat;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;

public class SmeEngine implements MessageListener, ResponseListener {

    private final static org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(SmeEngine.class);


    private boolean smsResponseMode = false;

    private DecimalFormatSymbols decimalFormatSymbols;
    private String numberFormatPattern = "#.##;-#.##";
    private String numberFormatNegativePattern = numberFormatPattern;

    private File responsePatternConfigFile;

    private String balanceResponsePattern = "{0} {1}";
    private String balanceNegativeResponsePattern;
    private String waitForSmsResponsePattern = "{0}";
    private String bannerAddPattern = "{0}\n{1}";
    private String errorPattern = "Error occurred";
    private boolean flashSmsEnabled = false;
    private String defaultEncoding = "rus";
    private RequestManager requestManager;

    private Map currency;

    private int ussdMaxLength = 67;

    private int maxProcessingRequests = 10000;

    private ThreadsPool threadsPool = null;
    private Multiplexor multiplexor = null;
    private OutgoingQueue outgoingQueue;


    private ProductivityControlObject requests;
    private ProductivityControlObject responses;
    private ProductivityControlObject waitForSmsResponses;


    public void init(Properties config) throws InitializationException {
        if (logger.isDebugEnabled()) logger.debug("UniBalance SME init started");

        /*
        numberFormatPattern = config.getProperty("balance.number.format.pattern", numberFormatPattern);
        numberFormatNegativePattern = config.getProperty("balance.number.format.negative.pattern", numberFormatNegativePattern);
        String decimalSeparator = config.getProperty("balance.number.format.decimal.separator", ".");
        if (decimalSeparator.length() == 0 || decimalSeparator.length() > 1) {
            throw new InitializationException("Invalid value for config parameter \"balance.number.format.decimal.separator\": " + decimalSeparator);
        }
        decimalFormatSymbols = new DecimalFormatSymbols(Locale.US);
        decimalFormatSymbols.setDecimalSeparator(decimalSeparator.charAt(0));
         */
        String responsePatternConfigFileName = config.getProperty("response.pattern.config.file", "response.pattern.properties");
        responsePatternConfigFile = new File(responsePatternConfigFileName);
        if (!responsePatternConfigFile.exists()) {
            responsePatternConfigFile = new File("config/" + responsePatternConfigFileName);
            if (!responsePatternConfigFile.exists()) {
                throw new InitializationException("Invalid value for config parameter \"response.pattern.config.file\": file not found");
            }
        }
        initResponsePatterns();

        long responsePatternConfigFileCheckInterval = 60000L;
        try {
            responsePatternConfigFileCheckInterval = Long.parseLong(config.getProperty("response.pattern.config.check.interval", Long.toString(60000L)));
        } catch (NumberFormatException e) {
            throw new InitializationException("Invalid value for config parameter \"response.pattern.config.check.interval\": " + config.getProperty("response.pattern.config.check.interval"));
        }
        (new ResponsePatternConfigController(responsePatternConfigFileCheckInterval)).startService();
        /*
     smsResponseMode = Boolean.valueOf(config.getProperty("sms.response.mode", "false")).booleanValue();
     flashSmsEnabled = Boolean.valueOf(config.getProperty("sms.response.flash", Boolean.toString(flashSmsEnabled))).booleanValue();
        */
        try {
            ussdMaxLength = Integer.parseInt(config.getProperty("unibalance.ussd.message.max.length", Integer.toString(ussdMaxLength)));
        } catch (NumberFormatException e) {
            throw new InitializationException("Invalid value for config parameter \"ussd.abonentRequest.max.length\": " + config.getProperty("ussd.message.max.length"));
        }

        try {
            maxProcessingRequests = Integer.parseInt(config.getProperty("max.processing.requests.count", Integer.toString(maxProcessingRequests)));
        } catch (NumberFormatException e) {
            throw new InitializationException("Invalid value for config parameter \"max.processing.requests.count\": " + config.getProperty("max.processing.requests.count"));
        }


        boolean productivityControllerEnabled = Boolean.valueOf(config.getProperty("productivity.controller.enabled", "true")).booleanValue();
        if (productivityControllerEnabled) {
            ProductivityController productivityController = new ProductivityController();
            productivityController.init(config);

            requests = new ProductivityControlObject("Requests");
            responses = new ProductivityControlObject("Responses");
            waitForSmsResponses = new ProductivityControlObject("Wait messages");

            productivityController.addControlObject(requests);
            productivityController.addControlObject(responses);
            productivityController.addControlObject(waitForSmsResponses);

            if (outgoingQueue != null && outgoingQueue.getMessageSender() != null) {
                productivityController.addControlObject(outgoingQueue.getMessageSender());
            }

            productivityController.startService();
        }

        defaultEncoding = config.getProperty("unibalance.default.encoding", defaultEncoding);

        /*!! Logic defined here!!*/
        requestManager = new MGRequestManager(this, config, threadsPool);

        if (logger.isDebugEnabled()) logger.debug("UniBalance SME init fineshed");
    }

    private void initResponsePatterns() throws InitializationException {
        Properties patternConfig;
        try {
            patternConfig = new Properties();
            patternConfig.load(new FileInputStream(responsePatternConfigFile));
        } catch (IOException e) {
            logger.error("Exception occured during loading response pattern configuration from " + responsePatternConfigFile.getName(), e);
            throw new InitializationException("Exception occured during loading response pattern configuration.", e);
        }

        balanceResponsePattern = patternConfig.getProperty("balance.response.pattern", balanceResponsePattern);
        balanceNegativeResponsePattern = patternConfig.getProperty("balance.negative.response.pattern");
        waitForSmsResponsePattern = patternConfig.getProperty("balance.wait.for.sms.response.pattern", waitForSmsResponsePattern);
        errorPattern = patternConfig.getProperty("balance.error.pattern", errorPattern);

        bannerAddPattern = patternConfig.getProperty("balance.banner.add.pattern", bannerAddPattern);

        currency = new HashMap();
        String defaultCurrency = patternConfig.getProperty("balance.currency.default", "usd");

        currency.put("default", defaultCurrency);

        String code;
        int n = 1;
        while ((code = patternConfig.getProperty("balance.currency." + n + ".code")) != null) {
            String value = patternConfig.getProperty("balance.currency." + n + ".name", "");
            currency.put(code, value);
            n++;
        }

    }

    public SmeEngine(Multiplexor multiplexor, OutgoingQueue messagesQueue, ThreadsPool threadsPool) {
        this.multiplexor = multiplexor;
        this.outgoingQueue = messagesQueue;
        this.threadsPool = threadsPool;
    }

    public String getName() {
        return "UniBalance SME";
    }

    public boolean handleMessage(Message msg) throws SMPPUserException {
        long requestTime = System.currentTimeMillis();
        try {
            if (msg.isReceipt()) {
                if (logger.isDebugEnabled())
                    logger.debug("SKIP MSG FROM #" + msg.getSourceAddress() + ". REASON: Receipt");
                sendDeliverSmResponse(msg, Data.ESME_ROK);
                return false;
            }
            if ((msg.getSourceAddress() == null || msg.getSourceAddress().trim().equals(""))) {
                if (logger.isDebugEnabled())
                    logger.debug("SKIP MSG FROM #" + msg.getSourceAddress() + ". REASON: Empty Source Address");
                sendDeliverSmResponse(msg, Data.ESME_ROK);
                return false;
            }
            /*
            if (msg.getEncoding() == Message.ENCODING_BINARY) {
              if (logger.isDebugEnabled())
                logger.debug("SKIP MSG FROM #" + msg.getSourceAddress() + ". REASON: Unsupported encoding: BINARY");
              sendDeliverSmResponse(msg, Data.ESME_ROK);
              return true;
            }
            */
            if (logger.isDebugEnabled())
                logger.debug("MSG handled. ConnID #" + msg.getConnectionId() + "; SeqN #" + msg.getSequenceNumber() + "; USSD #" + msg.getUssdServiceOp() + "; destination #" + msg.getDestinationAddress() + "; source #" + msg.getSourceAddress() + "; UserMessageReference:  " + msg.getUserMessageReference() + "; msg: " + msg.getMessageString());

            processIncomingMessage(msg, requestTime);
            return true;
        } catch (Exception e) { // logic exception (abonentRequest format)
            logger.error("Cannot handle MSG", e);
            sendDeliverSmResponse(msg, Data.ESME_RSYSERR);
            return false;
        }
    }

    public void handleResponse(PDU pdu) throws SMPPException {
        String responseType = "SUBMIT_SM_RESP";
        switch (pdu.getType()) {
            case Data.ENQUIRE_LINK_RESP:
                responseType = "ENQUIRE_LINK_RESP";
                break;
            case Data.DATA_SM_RESP:
                responseType = "DATA_SM_RESP";
                break;
        }

        if (pdu.getType() == Data.SUBMIT_SM_RESP || pdu.getType() == Data.DATA_SM_RESP) { // TODO: check for ussd_service_op

            if (logger.isDebugEnabled()) {
                logger.debug(responseType + " handled. ConnID #" + pdu.getConnectionId() + "; SeqN #" + pdu.getSequenceNumber() + "; Status #" + pdu.getStatus());
            }

            if (pdu.getStatusClass() == PDU.STATUS_CLASS_TEMP_ERROR) {
                outgoingQueue.updateOutgoingObject(pdu.getConnectionId(), pdu.getSequenceNumber(), pdu.getStatus());
            } else {
                requestManager.handleErrorPDU(pdu);
                outgoingQueue.removeOutgoingObject(pdu.getConnectionId(), pdu.getSequenceNumber(), pdu.getStatusClass());
            }
        }

    }

    public void sendDeliverSmResponse(Message msg, int status) {
        try {
            msg.setStatus(status);
            multiplexor.sendResponse(msg);
            if (logger.isDebugEnabled())
                logger.debug("Delivery response sent, destination #" + msg.getDestinationAddress() + "; source #" + msg.getSourceAddress() + "; status #" + msg.getStatus());
        } catch (SMPPException e) {
            logger.warn("Could not send delivery response.", e);
        }
    }

    public ProductivityControlObject getRequests() {
        return requests;
    }

    public ProductivityControlObject getResponses() {
        return responses;
    }

    public ProductivityControlObject getWaitForSmsResponses() {
        return waitForSmsResponses;
    }

    public String getWaitForSmsResponsePattern() {
        return waitForSmsResponsePattern;
    }

    public String getBalanceResponsePattern() {
        return balanceResponsePattern;
    }

    public String getDefaultEncoding() {
        return defaultEncoding;
    }

    public int getUssdMaxLength() {
        return ussdMaxLength;
    }

    public void sendMessage(Message msg, String connectionName) {
        try {
            if (outgoingQueue != null) {
                outgoingQueue.addOutgoingObject(new OutgoingObject(msg));
            } else {
                multiplexor.assingSequenceNumber(msg, connectionName);
                multiplexor.sendMessage(msg, false);
            }
            if (logger.isDebugEnabled())
                logger.debug("MSG sent. ConnID #" + msg.getConnectionId() + "; SeqN #" + msg.getSequenceNumber() + "; USSD #" + msg.getUssdServiceOp() + "; destination #" + msg.getDestinationAddress() + "; source #" + msg.getSourceAddress() + "; msg: " + msg.getMessageString());
        } catch (SMPPException e) {
            logger.error("Could not send abonentRequest", e);
        }
    }


    public Multiplexor getMultiplexor() {
        return multiplexor;
    }

    /*
       Передача сообщения RequestManager'у
     */
    private void processIncomingMessage(Message message, long abonentRequestTime) {
        requestManager.processIncomingMessage(message, abonentRequestTime);
        //TODO  max request
    }


    protected MessageFormat getMessageFormat(double balance) {
        if (balance >= 0 || balanceNegativeResponsePattern == null) {
            return new MessageFormat(balanceResponsePattern);
        } else {
            return new MessageFormat(balanceNegativeResponsePattern);
        }
    }

    protected String getCurrency(String code) {
        String result = (String) currency.get(code);
        if (result != null) {
            return result;
        }
        return (String) currency.get("default");
    }


    class ResponsePatternConfigController extends Thread {
        private boolean started = false;
        private Object monitor = new Object();
        private Object shutmonitor = new Object();

        private long pollingInterval = 60000L;
        private long fileLastModified;

        public ResponsePatternConfigController(long pollingInterval) {
            this.pollingInterval = pollingInterval;
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
                        initResponsePatterns();
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

}