package ru.sibinco.smpp.ub_sme;

import ru.aurorisoft.smpp.Message;

import java.util.Properties;


public class CollectorStateProcessor extends AbstractStateProcessor {
    private final static org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(CollectorStateProcessor.class);
    private String serviceAddress;
    private final String mgResultKey = "balance";
    private boolean smsResponseMode = false;
    private boolean flashSmsEnabled = false;
    private String errorPatternKey = "balance.error.pattern";
    private int ussdMaxLength = 67;
    private String waitForSmsResponseDefaultPattern = "{0}";
    private String keyForSmsResponsePattern = "balance.wait.for.sms.response.pattern";

    public CollectorStateProcessor(SmeEngine engine, States states) {
        super(engine, states);
    }


    public void init(Properties config, String name) {
        super.init(config, name);
        String prefix = "state.processor." + name;
        serviceAddress = config.getProperty(prefix + ".service.address");
        if (null == serviceAddress || "".equals(serviceAddress)) {
            throw new InitializationException("no " + prefix + ".service.address" + " in config");
        }
        smsResponseMode = Boolean.valueOf(config.getProperty("sms.response.mode", "false")).booleanValue();
        flashSmsEnabled = Boolean.valueOf(config.getProperty("sms.response.flash", Boolean.toString(flashSmsEnabled))).booleanValue();
        try {
            ussdMaxLength = Integer.parseInt(config.getProperty("ussd.message.max.length", Integer.toString(ussdMaxLength)));
        } catch (NumberFormatException e) {
            throw new InitializationException("Invalid value for config parameter \"ussd.message.max.length\": " + config.getProperty("ussd.message.max.length"));
        }

    }

    protected boolean checkState(State state) {        
        synchronized (state) {
            if (!serviceAddress.equals(state.getSourceMessage().getDestinationAddress())) {
                return false;
            }
        }
        return super.checkState(state);
    }

    protected void processState(State state) {
        synchronized (state) {
            if (!checkState(state)) {
                return;
            }
            state.addHistory(startHistoryRecord);
        }

        String balance = (String) state.getResultObject(mgResultKey);
        if (null == balance) {
            if (logger.isDebugEnabled()) {
                logger.debug("can't get get balance " + state.getSourceMessage().getSourceAddress());
            }
            sendErrorMessage(state);
            synchronized (state) {
                state.addHistory(endHistoryRecord);
            }
        } else {
            sendResponse(state, balance);
            synchronized (state) {
                state.addHistory(endHistoryRecord);

            }
        }
        synchronized (state) {
            state.setAbonentResponseTime(System.currentTimeMillis());
        }
        states.removeRequestState(state);

    }

    private void sendResponse(State state, String balance) {

        Message message = new Message();
        synchronized (state) {
            message.setSourceAddress(state.getSourceMessage().getDestinationAddress());
            message.setDestinationAddress(state.getSourceMessage().getSourceAddress());
            if (state.getSourceMessage().hasUssdServiceOp()&&!state.isUssdSessionClosed()) {
                message.setUssdServiceOp(Message.USSD_OP_PROC_SS_REQ_RESP);
            }
            message.setUserMessageReference(state.getSourceMessage().getUserMessageReference());
            message.setMessageString(balance);
            message.setType(Message.TYPE_SUBMIT);
            if (((message.getMessageString().length() > ussdMaxLength) || smsResponseMode) && !state.isUssdSessionClosed()) {
                sendWaitForSmsMessage(state);
                state.setUssdSessionClosed(true);
            }

            if (state.isUssdSessionClosed()) {
                message.setUssdServiceOp(-1);
                if (flashSmsEnabled) {
                    message.setDestAddrSubunit(1); // for Flash SMS
                }
            }
            smeEngine.sendResponseToAbonent(message, state.getSourceMessage().getConnectionName());
        }
    }

    protected void sendErrorMessage(State state) {
        Message message = new Message();
        String connectionName;
        synchronized (state) {
            message.setSourceAddress(state.getSourceMessage().getDestinationAddress());
            message.setDestinationAddress(state.getSourceMessage().getSourceAddress());

            if (state.getSourceMessage().hasUssdServiceOp() && !state.isUssdSessionClosed()) {
                message.setUssdServiceOp(Message.USSD_OP_PROC_SS_REQ_RESP);
            } else {
                if (flashSmsEnabled) {
                    message.setDestAddrSubunit(1); // for Flash SMS
                }
            }
            message.setUserMessageReference(state.getSourceMessage().getUserMessageReference());
            message.setMessageString(ResponsePatternManager.getResponsePatternManager().getPattern(errorPatternKey, "Error occurred"));
            message.setType(Message.TYPE_SUBMIT);
            connectionName = state.getSourceMessage().getConnectionName();
        }
        smeEngine.sendResponseToAbonent(message, connectionName);


    }

    private void sendWaitForSmsMessage(State state) {
        if (smsResponseMode) {
            return;
        }
        Message message = new Message();
        message.setSourceAddress(state.getSourceMessage().getDestinationAddress());
        message.setDestinationAddress(state.getSourceMessage().getSourceAddress());
        message.setUssdServiceOp(Message.USSD_OP_PROC_SS_REQ_RESP); 
        message.setUserMessageReference(state.getSourceMessage().getUserMessageReference());
        message.setMessageString(ResponsePatternManager.getResponsePatternManager().getPattern(keyForSmsResponsePattern, waitForSmsResponseDefaultPattern));
        message.setType(Message.TYPE_SUBMIT);
        smeEngine.sendResponseToAbonent(message, state.getSourceMessage().getConnectionName());
        if (smeEngine.getWaitForSmsResponses() != null) {
            smeEngine.getWaitForSmsResponses().count();
        }
    }
}
