package ru.sibinco.smpp.ub_sme.mg;

import com.logica.smpp.Data;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.PDU;
import ru.aurorisoft.smpp.SMPPException;
import ru.sibinco.smpp.ub_sme.*;

import java.util.Properties;


public class MGSender extends AbstractStateProcessor implements StateResponseProcessor {
    private final static org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(MGSender.class);
    private String serviceAddress;
    private String mgAddress;
    private final String startHistoryRecord = "mg_sender_start";
    private final String endHistoryRecord = "mg_sender_finish";
    private final String failedHistoryRecord = "mg_sender_failed";

    public MGSender(SmeEngine engine, States states) {
        super(engine, states);
    }

    public void init(Properties config, String name) throws InitializationException {
        super.init(config, name);

        String prefix = "state.processor." + name;
        serviceAddress = config.getProperty(prefix + ".service.address");
        if (null == serviceAddress || "".equals(serviceAddress)) {
            throw new InitializationException("no " + prefix + ".service.address" + " in config");
        }
        mgAddress = config.getProperty(prefix + ".mg_address");
        if (null == mgAddress || "".equals(mgAddress)) {
            throw new InitializationException("no " + prefix + ".mg_address" + " in config");
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
        String sourceAddress;
        String abonent;
        synchronized (state) {
            if (!checkState(state)) {
                return;
            }
            if (logger.isDebugEnabled()) {
                logger.debug("Process state " );
            }
            state.addHistory(startHistoryRecord);
            sourceAddress = state.getSourceMessage().getSourceAddress();
            abonent = state.getSourceMessage().getSourceAddress();
        }
        String messageBody = abonent + "*balance";
        Message message = new Message();
        message.setSourceAddress(sourceAddress);
        message.setDestinationAddress(mgAddress);
        message.setMessageString(messageBody);
        message.setEsmClass((byte) (Data.SM_FORWARD_MODE));
        message.setType(Message.TYPE_SUBMIT);


        try {
            smeEngine.assingSequenceNumber(message, state);
        } catch (SMPPException e) {
            logger.error("Could not send MG request", e);
            synchronized (state) {
                state.addHistory(failedHistoryRecord);
            }
            return;
        }
        //TODO delete it
/*
        Message testMessage = new Message();
        testMessage.setSourceAddress(mgAddress);
        testMessage.setDestinationAddress(state.getSourceMessage().getSourceAddress());
        testMessage.setMessageString("123");
        try {
            Thread.sleep(10000);
        } catch (InterruptedException e) {
            e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
        }
        states.addRequestState(new State(testMessage,System.currentTimeMillis()));
        state.addHistory(endHistoryRecord);
        if(true) return;
  */
        synchronized (state) {
            boolean status = smeEngine.sendMessage(message);
            if (status) {
                state.addHistory(endHistoryRecord);
                smeEngine.addResponseHandler(smeEngine.getSequenceNumber(message), new PDUHandle(this, state));
                if (logger.isDebugEnabled()) {
                    logger.debug("Send Mg request for abonent " + state.getSourceMessage().getSourceAddress());
                }
            } else {
                state.addHistory(failedHistoryRecord);
                logger.error("Could not send MG request");
            }
        }


    }

    public void handleResponse(PDU pdu, State state) {
        smeEngine.removeResponseHandler(smeEngine.getSequenceNumber(pdu));
        synchronized (state) {
            state.addHistory(failedHistoryRecord);
            if (logger.isDebugEnabled()) {
                logger.debug("PDU error");
            }
        }
        smeEngine.removeOutgoingObject(pdu);
    }


}
