package ru.sibinco.smpp.ub_sme.outgoing;

public interface OutgoingQueueProxy {
    public void addOutgoingObject(Response response);

    public boolean registerMessageStatusListener(MessageStatusListener listener);
}
