package ru.sibinco.smpp.ub_sme;

import com.logica.smpp.Data;
import com.lorissoft.advertising.syncclient.AdvertisingClientImpl;
import com.lorissoft.advertising.syncclient.IAdvertisingClient;
import ru.aurorisoft.smpp.*;
import ru.sibinco.smpp.ub_sme.mg.MGState;
import ru.sibinco.smpp.ub_sme.util.Convertor;
import ru.sibinco.smpp.ub_sme.util.Utils;
import ru.sibinco.util.threads.ThreadsPool;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.text.DecimalFormatSymbols;
import java.text.MessageFormat;
import java.util.*;

public class SmeEngine implements MessageListener, ResponseListener {

    private final static org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(SmeEngine.class);

    private String mgAddress = null;


    private String bannerEngineServiceName = "UniBalance";
    private int bannerEngineClientID = 1;
    private int bannerEngineTransportType = 1;
    private int bannerEngineCharSet = 1;

    private boolean bannerEngineClientEnabled = false;
    private IAdvertisingClient bannerEngineClient;
    private String bannerEngineClientHost = "localhost";
    private String bannerEngineClientPort = "8555";
    private String bannerEngineClientTimeout = "1000";
    private String bannerEngineClientReconnectTimeout = "1000";

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

    private Map currency;

    private int ussdMaxLength = 67;
    private long ussdSessionTimeout = 5000L;

    private int maxProcessingRequests = 10000;
    private int expireTime = 5000;

    private ThreadsPool threadsPool = null;
    private Multiplexor multiplexor = null;
    private OutgoingQueue outgoingQueue;
    private Map mgRequests = new HashMap();
    private Map mgAbonentRequests = Collections.synchronizedMap(new HashMap());

    private ProductivityControlObject requests;
    private ProductivityControlObject responses;
    private ProductivityControlObject waitForSmsResponses;


    private int bannerEngineTransactionId;
    private final Object bannerEngineTransactionIdSyncMonitor = new Object();

    public int getExpireTime() {
        return expireTime;
    }

    public void init(Properties config) throws InitializationException {
        if (logger.isDebugEnabled()) logger.debug("UniBalance SME init started");

        mgAddress = config.getProperty("mg.address", "");
        if (mgAddress.length() == 0) {
            throw new InitializationException("Mandatory config parameter \"mg.address\" is missed");
        }


        numberFormatPattern = config.getProperty("balance.number.format.pattern", numberFormatPattern);
        numberFormatNegativePattern = config.getProperty("balance.number.format.negative.pattern", numberFormatNegativePattern);
        String decimalSeparator = config.getProperty("balance.number.format.decimal.separator", ".");
        if (decimalSeparator.length() == 0 || decimalSeparator.length() > 1) {
            throw new InitializationException("Invalid value for config parameter \"balance.number.format.decimal.separator\": " + decimalSeparator);
        }
        decimalFormatSymbols = new DecimalFormatSymbols(Locale.US);
        decimalFormatSymbols.setDecimalSeparator(decimalSeparator.charAt(0));

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

        smsResponseMode = Boolean.valueOf(config.getProperty("sms.response.mode", "false")).booleanValue();
        flashSmsEnabled = Boolean.valueOf(config.getProperty("sms.response.flash", Boolean.toString(flashSmsEnabled))).booleanValue();

        try {
            ussdMaxLength = Integer.parseInt(config.getProperty("ussd.message.max.length", Integer.toString(ussdMaxLength)));
        } catch (NumberFormatException e) {
            throw new InitializationException("Invalid value for config parameter \"ussd.abonentRequest.max.length\": " + config.getProperty("ussd.message.max.length"));
        }
        try {
            ussdSessionTimeout = Long.parseLong(config.getProperty("ussd.session.timeout", Long.toString(ussdSessionTimeout)));
        } catch (NumberFormatException e) {
            throw new InitializationException("Invalid value for config parameter \"ussd.session.timeout\": " + config.getProperty("ussd.session.timeout"));
        }
        long requestStatesControllerPollingInterval;
        try {
            requestStatesControllerPollingInterval = Long.parseLong(config.getProperty("request.states.controller.polling.interval", Long.toString(ussdSessionTimeout)));
        } catch (NumberFormatException e) {
            throw new InitializationException("Invalid value for config parameter \"request.states.controller.polling.interval\": " + config.getProperty("request.states.controller.polling.interval"));
        }
        try {
            maxProcessingRequests = Integer.parseInt(config.getProperty("max.processing.requests.count", Integer.toString(maxProcessingRequests)));
        } catch (NumberFormatException e) {
            throw new InitializationException("Invalid value for config parameter \"max.processing.requests.count\": " + config.getProperty("max.processing.requests.count"));
        }

        bannerEngineClientEnabled = Boolean.valueOf(config.getProperty("banner.engine.client.enabled", Boolean.toString(bannerEngineClientEnabled))).booleanValue();
        if (bannerEngineClientEnabled) {

            bannerEngineServiceName = config.getProperty("banner.engine.service.name", bannerEngineServiceName);
            if (bannerEngineServiceName.length() == 0) {
                throw new InitializationException("Mandatory config parameter \"banner.engine.service.name\" is missed");
            }
            try {
                bannerEngineClientID = Integer.parseInt(config.getProperty("banner.engine.client.id", Integer.toString(bannerEngineClientID)));
            } catch (NumberFormatException e) {
                throw new InitializationException("Invalid value for config parameter \"banner.engine.client.id\": " + config.getProperty("banner.engine.client.id"));
            }
            try {
                bannerEngineTransportType = Integer.parseInt(config.getProperty("banner.engine.transport.type", Integer.toString(bannerEngineTransportType)));
            } catch (NumberFormatException e) {
                throw new InitializationException("Invalid value for config parameter \"banner.engine.transport.type\": " + config.getProperty("banner.engine.transport.type"));
            }
            try {
                bannerEngineCharSet = Integer.parseInt(config.getProperty("banner.engine.charset", Integer.toString(bannerEngineCharSet)));
            } catch (NumberFormatException e) {
                throw new InitializationException("Invalid value for config parameter \"banner.engine.charset\": " + config.getProperty("banner.engine.charset"));
            }

            bannerEngineClientHost = config.getProperty("banner.engine.client.host", bannerEngineClientHost);
            if (bannerEngineClientHost.length() == 0) {
                throw new InitializationException("Mandatory config parameter \"banner.engine.client.host\" is missed");
            }
            bannerEngineClientPort = config.getProperty("banner.engine.client.port", bannerEngineClientPort);
            if (bannerEngineClientPort.length() == 0) {
                throw new InitializationException("Mandatory config parameter \"banner.engine.client.port\" is missed");
            }
            bannerEngineClientTimeout = config.getProperty("banner.engine.client.timeout", bannerEngineClientTimeout);
            if (bannerEngineClientTimeout.length() == 0) {
                throw new InitializationException("Mandatory config parameter \"banner.engine.client.timeout\" is missed");
            }
            bannerEngineClientReconnectTimeout = config.getProperty("banner.engine.client.reconnect.timeout", bannerEngineClientReconnectTimeout);
            if (bannerEngineClientReconnectTimeout.length() == 0) {
                throw new InitializationException("Mandatory config parameter \"banner.engine.client.reconnect.timeout\" is missed");
            }
            Properties bannerEngineClientConfig = new Properties();
            bannerEngineClientConfig.put("ip", bannerEngineClientHost);
            bannerEngineClientConfig.put("port", bannerEngineClientPort);
            bannerEngineClientConfig.put("CLIENTTIMEOUT", bannerEngineClientTimeout);
            bannerEngineClientConfig.put("WATCHDOGSLEEP", bannerEngineClientReconnectTimeout);

            bannerEngineClient = new AdvertisingClientImpl();
            bannerEngineClient.init(bannerEngineClientConfig);
        }
        try {
            expireTime = Integer.parseInt(config.getProperty("expire.time"));
        } catch (NumberFormatException e) {
            throw new InitializationException("Invalid value for config parameter \"expire.time\": " + config.getProperty("expire.time"));
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

        defaultEncoding = config.getProperty("default.encoding", defaultEncoding);
        // new RequestStatesController(requestStatesControllerPollingInterval).startService();

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
                logger.debug("MSG handled. ConnID #" + msg.getConnectionId() + "; SeqN #" + msg.getSequenceNumber() + "; USSD #" + msg.getUssdServiceOp() + "; destination #" + msg.getDestinationAddress() + "; source #" + msg.getSourceAddress() + "; msg: " + msg.getMessageString());

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

                MGState state = (MGState) mgRequests.get(new Long(((long) pdu.getConnectionId()) << 32 | pdu.getSequenceNumber()));
                if (state != null && pdu.getStatus() != PDU.STATUS_CLASS_NO_ERROR) {
                    mgRequests.remove(new Long(((long) pdu.getConnectionId()) << 32 | pdu.getSequenceNumber()));
                    state.setMgState(MGState.MG_ERR);
                    state.closeProcessing();
                }


                outgoingQueue.removeOutgoingObject(pdu.getConnectionId(), pdu.getSequenceNumber(), pdu.getStatusClass());
            }
        }

    }

    private void sendDeliverSmResponse(Message msg, int status) {
        try {
            msg.setStatus(status);
            multiplexor.sendResponse(msg);
            if (logger.isDebugEnabled())
                logger.debug("Delivery response sent, destination #" + msg.getDestinationAddress() + "; source #" + msg.getSourceAddress() + "; status #" + msg.getStatus());
        } catch (SMPPException e) {
            logger.warn("Could not send delivery response.", e);
        }
    }

    public void sendWaitForSmsMessage(MGState state) {
        /*
        if (smsResponseMode) {
          return;
        }
        */
        Message message = prepareMessage(state);
        message.setMessageString(waitForSmsResponsePattern);
        sendMessage(message, state.getAbonentRequest().getConnectionName());
        if (waitForSmsResponses != null) {
            waitForSmsResponses.count();
        }
    }

    public void sendErrorSmsMessage(MGState state) {
        /*
        if (smsResponseMode) {
          return;
        }
        */
        Message message = prepareMessage(state);
        message.setMessageString(errorPattern);
        sendMessage(message, state.getAbonentRequest().getConnectionName());
        if (waitForSmsResponses != null) {
            waitForSmsResponses.count();
        }
    }

    public Message prepareMessage(MGState state) {
        Message message = new Message();
        message.setSourceAddress(state.getAbonentRequest().getDestinationAddress());
        message.setDestinationAddress(state.getAbonentRequest().getSourceAddress());
        message.setUssdServiceOp(Message.USSD_OP_PROC_SS_REQ_RESP);
        message.setUserMessageReference(state.getAbonentRequest().getUserMessageReference());
        message.setType(Message.TYPE_SUBMIT);
        return message;
    }

    public void sendResponse(MGState state) {
        boolean sms = false;
        if (state.getMessage().length() > ussdMaxLength) {
            sendWaitForSmsMessage(state);
            sms = true;
        }
        Message msg = new Message();
        msg.setSourceAddress(state.getAbonentRequest().getDestinationAddress());
        msg.setDestinationAddress(state.getAbonentRequest().getSourceAddress());
        msg.setMessageString(state.getMessage());
        if (state.getAbonentRequest().hasUssdServiceOp() && !sms) {
            msg.setUssdServiceOp(Message.USSD_OP_PROC_SS_REQ_RESP);
        }
        msg.setUserMessageReference(state.getAbonentRequest().getUserMessageReference());
        msg.setType(Message.TYPE_SUBMIT);
        if (flashSmsEnabled) {
            msg.setDestAddrSubunit(1); // for Flash SMS
        }
        sendMessage(msg, state.getAbonentRequest().getConnectionName());
        if (responses != null) {
            responses.count();
        }

    }

    private void sendMessage(Message msg, String connectionName) {
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

    public String prepareBalanceMessage(String balance, String banner, String encoding) {
        if (banner == null) {
            banner = "";
        }
        if (encoding == null) {
            encoding = defaultEncoding;
        }
        MessageFormat mf = new MessageFormat(balanceResponsePattern);
        String messageString = mf.format(new String[]{balance, banner});
        if ("tr".equalsIgnoreCase(encoding) ||
                "translit".equalsIgnoreCase(encoding) ||
                "trl".equalsIgnoreCase(encoding)) {
            messageString = Convertor.translit(messageString);
        }
        return messageString;
    }


    private void processIncomingMessage(Message message, long abonentRequestTime) {

        if (message.getSourceAddress().equals(mgAddress)) { // abonent request
            if (logger.isDebugEnabled())
                logger.debug("Got request from " + message.getSourceAddress());
            MGState state = (MGState) mgAbonentRequests.remove(Utils.trimAbonent(message.getDestinationAddress()));
            if (state.isExpired()) {
                sendDeliverSmResponse(message, Data.ESME_RSYSERR);
            }
            state.setMgBalance(message.getMessageString());
            state.setMgState(MGState.MG_OK);
            state.closeProcessing();
        } else {
            if (requests != null) {
                requests.count();
            }
            MGState state = new MGState(threadsPool, message);
            state.startProcessing();
        }
        //TODO  max request

    }


    public void sendMgRequest(MGState state, Message msg) {
        try {
            multiplexor.assingSequenceNumber(msg, state.getAbonentRequest().getConnectionName());
            mgRequests.put(new Long(((long) msg.getConnectionId()) << 32 | msg.getSequenceNumber()), state);
            mgAbonentRequests.put(Utils.trimAbonent(state.getAbonentRequest().getSourceAddress()), state);
            multiplexor.sendMessage(msg, false);
            if (logger.isDebugEnabled())
                logger.debug("MSG sent. ConnID #" + msg.getConnectionId() + "; SeqN #" + msg.getSequenceNumber() + "; USSD #" + msg.getUssdServiceOp() + "; destination #" + msg.getDestinationAddress() + "; source #" + msg.getSourceAddress() + "; msg: " + msg.getMessageString());
        } catch (SMPPException e) {
            logger.error("Could not send MG request", e);
            state.setMgState(MGState.MG_ERR);
            mgRequests.remove(new Long(((long) msg.getConnectionId()) << 32 | msg.getSequenceNumber()));
            state.closeProcessing();
        }
    }


    public String getBanner(MGState state) {
        String abonent = state.getAbonentRequest().getSourceAddress();
        byte[] banner = null;

        String encoding = "UTF-16BE";
        if (abonent.startsWith("+")) {
            abonent = ".1.1." + abonent.substring(1);
        } else if (!abonent.startsWith(".")) {
            abonent = ".1.1." + abonent;
        }
        int transactionId;
        synchronized (bannerEngineTransactionIdSyncMonitor) {
            transactionId = bannerEngineTransactionId++;
        }
        //  state.setBannerRequested();
        banner = bannerEngineClient.getLikelyBanner(abonent.getBytes(), abonent.getBytes().length, bannerEngineServiceName.getBytes(), bannerEngineTransportType, 140, bannerEngineCharSet, bannerEngineClientID, transactionId);

        if (banner == null) {
            return null;
        }

        try {
            return new String(banner, encoding);
        } catch (UnsupportedEncodingException e) {
            logger.error("Unsupported encoding: " + encoding, e);
            return null;
        }
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


    public String getMgAddress() {
        return mgAddress;
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
