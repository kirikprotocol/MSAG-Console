package ru.sibinco.smpp.ub_sme.mg;

import com.logica.smpp.Data;
import ru.aurorisoft.smpp.Message;
import ru.sibinco.smpp.ub_sme.SmeEngine;


public class MgRequestStateProcessor implements Runnable {
    private final static org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(MgRequestStateProcessor.class);
    protected MGState state;
    protected SmeEngine smeEngine;
    protected MGRequestManager requestManager;

    public MgRequestStateProcessor(MGState state, MGRequestManager requestManager) {
        this.state = state;
        this.requestManager = requestManager;
    }

    public void run() {
        Message message = new Message();
        message.setUserMessageReference(state.getAbonentRequest().getUserMessageReference());
        message.setMessageString(state.getAbonentRequest().getSourceAddress() + "*balance");
        message.setEsmClass((byte) (Data.SM_FORWARD_MODE));
        message.setType(Message.TYPE_SUBMIT);
        state.setMgState(MGState.MG_WAIT_RESP);
        requestManager.sendMgRequest(message, state);
        state.closeProcessing();
    }


}
