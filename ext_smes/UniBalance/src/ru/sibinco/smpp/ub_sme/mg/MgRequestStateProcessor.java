package ru.sibinco.smpp.ub_sme.mg;

import com.logica.smpp.Data;
import ru.aurorisoft.smpp.Message;
import ru.sibinco.smpp.ub_sme.Sme;
import ru.sibinco.smpp.ub_sme.SmeEngine;


public class MgRequestStateProcessor implements Runnable {
    protected MGState state;
    protected SmeEngine smeEngine;

    public MgRequestStateProcessor(MGState state) {
        this.state = state;
        smeEngine = Sme.getSmeEngine();
    }

    public void run() {
        Message message = new Message();
        message.setSourceAddress(state.getAbonentRequest().getSourceAddress());
        message.setDestinationAddress(smeEngine.getMgAddress());
        message.setUserMessageReference(state.getAbonentRequest().getUserMessageReference());
        message.setMessageString(state.getAbonentRequest().getSourceAddress() + "*balance");
        message.setEsmClass((byte) (Data.SM_FORWARD_MODE));
        message.setType(Message.TYPE_SUBMIT);
        state.setMgState(MGState.MG_WAIT_RESP);      
        smeEngine.sendMgRequest(state, message);
        state.closeProcessing();
    }

}
