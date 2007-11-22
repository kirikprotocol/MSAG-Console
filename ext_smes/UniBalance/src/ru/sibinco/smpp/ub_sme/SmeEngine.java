package ru.sibinco.smpp.ub_sme;

import com.logica.smpp.Data;
import ru.aurorisoft.smpp.*;
import ru.sibinco.smpp.ub_sme.outgoing.OutgoingObject;
import ru.sibinco.smpp.ub_sme.outgoing.OutgoingQueue;
import ru.sibinco.smpp.ub_sme.productivity.ProductivityControlObject;
import ru.sibinco.smpp.ub_sme.productivity.ProductivityController;

import java.io.File;
import java.util.*;


public class SmeEngine implements MessageListener, ResponseListener {
    private final static org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(SmeEngine.class);
    private States states = new States();
    private Multiplexor multiplexor = null;
    private OutgoingQueue outgoingQueue;
    private int maxProcessingRequests = 10000;
    private Map responseSequenceMap = Collections.synchronizedMap(new HashMap());
    private ProductivityControlObject requests;
    private ProductivityControlObject responses;
    private ProductivityControlObject waitForSmsResponses;

    public void init(Properties config) {


        String responsePatternConfigFileName = config.getProperty("response.pattern.config.file", "response.pattern.properties");
        File responsePatternConfigFile = new File(responsePatternConfigFileName);
        if (!responsePatternConfigFile.exists()) {
            responsePatternConfigFile = new File("config/" + responsePatternConfigFileName);
            if (!responsePatternConfigFile.exists()) {
                throw new InitializationException("Invalid value for config parameter \"response.pattern.config.file\": file not found");
            }
        }

        long responsePatternConfigFileCheckInterval = 60000L;
        try {
            responsePatternConfigFileCheckInterval = Long.parseLong(config.getProperty("response.pattern.config.check.interval", Long.toString(60000L)));
        } catch (NumberFormatException e) {
            throw new InitializationException("Invalid value for config parameter \"response.pattern.config.check.interval\": " + config.getProperty("response.pattern.config.check.interval"));
        }
        ResponsePatternManager.getResponsePatternManager().loadPatterns(responsePatternConfigFile);
        (new ResponsePatternConfigControllerThread(responsePatternConfigFileCheckInterval, responsePatternConfigFile)).startService();

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
        try {
            Enumeration enum = config.keys();
            while (enum.hasMoreElements()) {
                String key = (String) enum.nextElement();
                if (key.startsWith("state.processor.class")) {
                    String processorName = key.substring("state.processor.class".length() + 1);
                    String value = config.getProperty(key);
                    AbstractStateProcessor processor = (AbstractStateProcessor) Class.forName(value).getConstructor(new Class[]{SmeEngine.class, States.class}).newInstance(new Object[]{this, states});
                    processor.init(config, processorName);
                    processor.startProcessorThreads();
                }

            }
        } catch (Exception e) {
            throw new InitializationException(e);
        }

        try {
            maxProcessingRequests = Integer.parseInt(config.getProperty("max.processing.requests.count", Integer.toString(maxProcessingRequests)));
        } catch (NumberFormatException e) {
            throw new InitializationException("Invalid value for config parameter \"max.processing.requests.count\": " + config.getProperty("max.processing.requests.count"));
        }

        new RequestStatesController(config, this).startService();
        if (logger.isDebugEnabled()) logger.debug("UniBalance SME init fineshed");
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
                updateOutgoingObject(pdu);
            } else {
                if (responseSequenceMap.containsKey(getSequenceNumber(pdu))) {
                    PDUHandle pduHandle = (PDUHandle) responseSequenceMap.get(getSequenceNumber(pdu));
                    pduHandle.handleResponse(pdu);
                }
                removeOutgoingObject(pdu);
            }
        }
    }

    public SmeEngine(Multiplexor multiplexor, OutgoingQueue messagesQueue) {
        this.multiplexor = multiplexor;
        this.outgoingQueue = messagesQueue;
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

            if (logger.isDebugEnabled())
                logger.debug("MSG handled. ConnID #" + msg.getConnectionId() + "; SeqN #" + msg.getSequenceNumber() + "; USSD #" + msg.getUssdServiceOp() + "; destination #" + msg.getDestinationAddress() + "; source #" + msg.getSourceAddress() + "; msg: " + msg.getMessageString());


            if (states.size() >= maxProcessingRequests) {
                if (logger.isDebugEnabled())
                    logger.debug("Request rejected because max processing requests count is reached");
                sendDeliverSmResponse(msg, Data.ESME_RMSGQFUL);
                return false;
            }
            if (requests != null) {
                requests.count();
            }
            processIncomingMessage(msg, requestTime);
            return true;
        } catch (Exception e) { // logic exception (message format)
            logger.error("Cannot handle MSG", e);
            sendDeliverSmResponse(msg, Data.ESME_RSYSERR);
            return false;
        }
    }

    private void processIncomingMessage(Message message, long abonentRequestTime) {
        State state;
        state = new State(message, abonentRequestTime);
        states.addRequestState(state);
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


    public boolean sendMessage(Message message) {
        try {
            multiplexor.sendMessage(message, false);
        } catch (SMPPException e) {
            logger.error("Could not send MG request", e);
            return false;
        }
        return true;
    }

    public void sendResponseToAbonent(Message msg, String connectionName) {
        if (responses != null) {
            responses.count();
        }
        sendMessage(msg, connectionName);
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
            logger.error("Could not send message", e);
        }
    }

    public void updateOutgoingObject(PDU pdu) {
        outgoingQueue.updateOutgoingObject(pdu.getConnectionId(), pdu.getSequenceNumber(), pdu.getStatus());
    }

    public void removeOutgoingObject(PDU pdu) {
        outgoingQueue.removeOutgoingObject(pdu.getConnectionId(), pdu.getSequenceNumber(), pdu.getStatusClass());
    }

    public void addResponseHandler(Long sequenceNumber, PDUHandle pduHandle) {
        responseSequenceMap.put(sequenceNumber, pduHandle);
    }

    public void removeResponseHandler(Long sequenceNumber) {
        responseSequenceMap.remove(sequenceNumber);
    }

    public Long getSequenceNumber(PDU pdu) {
        if (pdu == null) {
            return null;
        }
        return new Long(((long) pdu.getConnectionId()) << 32 | pdu.getSequenceNumber());
    }

    public States getStates() {
        return states;
    }

    public void assingSequenceNumber(Message msg, State state) throws SMPPException {
        multiplexor.assingSequenceNumber(msg, state.getSourceMessage().getConnectionName());
    }

    public ProductivityControlObject getWaitForSmsResponses() {
        return waitForSmsResponses;
    }
}
