package ru.sibinco.smpp.ub_sme;

import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.PDU;

/**
 * User: pasha
 * Date: 14.01.2008
 * Time: 16:33:34
 */
public interface RequestManager {

    public void processIncomingMessage(Message message, long abonentRequestTime);

    public void handleErrorPDU(PDU pdu);
}
