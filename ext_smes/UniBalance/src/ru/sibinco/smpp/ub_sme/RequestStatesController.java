package ru.sibinco.smpp.ub_sme;

import ru.aurorisoft.smpp.Message;

import java.util.Properties;


class RequestStatesController extends Thread {
    private final static org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(RequestStatesController.class);
    private boolean started = false;
    private long requestStatesControllerPollingInterval= 5000L;
    private final Object monitor = new Object();
    private final Object shutmonitor = new Object();
    private boolean smsResponseMode = false;
    private SmeEngine engine;
    private States states;
    private long ussdSessionTimeout = 5000L;
    private String waitForSmsResponseDefaultPattern = "{0}";
    private String keyForSmsResponsePattern = "balance.wait.for.sms.response.pattern";

    public RequestStatesController(Properties config, SmeEngine engine) {
        setName("RequestStatesController");

        long requestStatesControllerPollingInterval = 5000L;
        try {
            requestStatesControllerPollingInterval = Long.parseLong(config.getProperty("request.states.controller.polling.interval", String.valueOf(requestStatesControllerPollingInterval)));
        } catch (NumberFormatException e) {
            throw new InitializationException("Invalid value for config parameter \"request.states.controller.polling.interval\": " + config.getProperty("request.states.controller.polling.interval"));
        }
        try {
            ussdSessionTimeout = Long.parseLong(config.getProperty("ussd.session.timeout", Long.toString(ussdSessionTimeout)));
        } catch (NumberFormatException e) {
            throw new InitializationException("Invalid value for config parameter \"ussd.session.timeout\": " + config.getProperty("ussd.session.timeout"));
        }
        this.states = engine.getStates();
        this.engine = engine;
        smsResponseMode = Boolean.valueOf(config.getProperty("sms.response.mode", "false")).booleanValue();
        if (logger.isInfoEnabled())
            logger.info(getName() + " created.");
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

    public boolean isStarted() {
        return started;
    }

    public void run() {
        while (started) {
            processRequestStates();
            synchronized (monitor) {
                if (!started) break;
                try {
                    monitor.wait(requestStatesControllerPollingInterval);
                } catch (InterruptedException e) {
                    logger.warn(getName() + " was interrupted.", e);
                }
            }
        }
        synchronized (shutmonitor) {
            shutmonitor.notifyAll();
        }
    }

    public void processRequestStates() {
        State state;
        if (logger.isDebugEnabled()) {
            logger.debug("Processing requests count: " + states.size());
        }
        for (int i = 0; i < states.size(); i++) {
            try {
                state = (State) states.get(i);
            } catch (IndexOutOfBoundsException e) {
                return;
            }
            if (state.getAbonentRequestTime() + ussdSessionTimeout > System.currentTimeMillis()) {
                break;
            }
            boolean sendWaitMessage = false;
            synchronized (state) {
                /*
                if (state.getAbonentRequestTime() + requestLifeTime <= System.currentTimeMillis() && !state.isClosed()) {
                  if (logger.isDebugEnabled()) logger.debug(state.getAbonentRequest().getSourceAddress() + " request state expired.");
                  if (!state.isUssdSessionClosed()) {
                    sendDeliverSmResponse(state.getAbonentRequest(), Data.ESME_RSYSERR);
                  }
                  state.setClosed(true);
                  states.remove(0);
                  continue;
                }
                */
                if (!state.isUssdSessionClosed()) {
                    sendWaitMessage = true;
                    state.setUssdSessionClosed(true);
                }
            }
            if (sendWaitMessage) {
                sendWaitForSmsMessage(state);
            }
        }
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
        engine.sendResponseToAbonent(message, state.getSourceMessage().getConnectionName());
        if (engine.getWaitForSmsResponses() != null) {
            engine.getWaitForSmsResponses().count();
        }
    }
}




