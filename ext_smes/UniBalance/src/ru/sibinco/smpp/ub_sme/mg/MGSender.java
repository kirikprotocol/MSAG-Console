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
                logger.debug("Process state: "+state);
            }
            state.addHistory(startHistoryRecord);
            sourceAddress = state.getSourceMessage().getSourceAddress();
        }
        String messageBody = sourceAddress + "*balance";
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
                state.addHistory(errorHistoryRecord);
            }
            return;
        }
       synchronized (state) {
           boolean status = smeEngine.sendMessage(message);
            if (status) {
                state.addHistory(endHistoryRecord);
                smeEngine.addResponseHandler(smeEngine.getSequenceNumber(message), new PDUHandle(this, state));
                if (logger.isDebugEnabled()) {
                        logger.debug("MSG sent. ConnID #" + message.getConnectionId() + "; SeqN #" + message.getSequenceNumber() + "; USSD #" + message.getUssdServiceOp() + "; destination #" + message.getDestinationAddress() + "; source #" + message.getSourceAddress() + "; msg: " + message.getMessageString());
                }
            } else {
                state.addHistory(errorHistoryRecord);
                logger.error("Could not send MG request");
            }
        }


    }

    public void handleResponse(PDU pdu, State state) {
        smeEngine.removeResponseHandler(smeEngine.getSequenceNumber(pdu));
        synchronized (state) {
            state.addHistory(errorHistoryRecord);
            if (logger.isDebugEnabled()) {
                logger.debug("PDU error, status class:"+pdu.getStatusClass());
            }
        }
    }


}
