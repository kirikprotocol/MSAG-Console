package ru.sibinco.smpp.ub_sme;

import ru.aurorisoft.smpp.PDU;

public class PDUHandle {
    protected StateResponseProcessor processor;
    protected State state;

    public PDUHandle(StateResponseProcessor processor, State state) {
        this.processor = processor;
        this.state = state;
    }

    public void setProcessor(StateResponseProcessor processor) {
        this.processor = processor;
    }

    public void handleResponse(PDU pdu) {
        processor.handleResponse(pdu, state);
    }

    public void setState(State state) {
        this.state = state;
    }

    public StateResponseProcessor getProcessor() {
        return processor;
    }

    public State getState() {
        return state;
    }

}
