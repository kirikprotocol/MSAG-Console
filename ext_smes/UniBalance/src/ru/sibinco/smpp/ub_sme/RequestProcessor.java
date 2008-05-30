package ru.sibinco.smpp.ub_sme;

import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.PDU;
import ru.sibinco.util.threads.ThreadsPool;

import java.util.Properties;

/**
 * User: pasha
 * Date: 14.01.2008
 * Time: 16:33:34
 */
public interface RequestProcessor {

  public void processIncomingMessage(Message message, long abonentRequestTime);

  public void handleErrorPDU(PDU pdu);

  public void setThreadsPool(ThreadsPool threadsPool);

  public void setSmeEngine(SmeEngine smeEngine);

  public void init(Properties config) throws InitializationException;

}
