package ru.sibinco.calendarsme.engine;

import com.logica.smpp.Data;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.Multiplexor;
import ru.aurorisoft.smpp.SMPPException;
import ru.sibinco.calendarsme.InitializationException;
import ru.sibinco.calendarsme.engine.calendar.CalendarSmeEngine;
import ru.sibinco.calendarsme.engine.secret.SecretRequestProcessor;
import ru.sibinco.calendarsme.network.IncomingQueue;
import ru.sibinco.calendarsme.network.OutgoingQueue;
import ru.sibinco.calendarsme.utils.Service;

/**
 * User: artem
 * Date: Jul 27, 2006
 */

public class SmeEngine extends Service {
  private static final org.apache.log4j.Category Log = org.apache.log4j.Category.getInstance(SmeEngine.class);
  private static final String NAME = "SmeEngine";

  private final IncomingQueue inQueue;
  private final Multiplexor multiplexor;

  private final CalendarSmeEngine calendarSmeEngine;
  private final SecretRequestProcessor secretRequestProcessor;

  public SmeEngine(final IncomingQueue inQueue, final OutgoingQueue outQueue, final Multiplexor multiplexor) {

    super(Log);
    if (inQueue == null)
      throw new InitializationException(NAME + ": incoming queue is not specified");
    if (multiplexor == null)
      throw new InitializationException(NAME + ": multiplexor is not specified");

    this.inQueue = inQueue;
    this.multiplexor = multiplexor;

    calendarSmeEngine = new CalendarSmeEngine(outQueue, multiplexor);
    secretRequestProcessor = new SecretRequestProcessor(outQueue, multiplexor);

    Log.info(NAME + ": initialization ok");
  }

  public void iterativeWork() {
    final Message message = inQueue.getIncomingObject().getMessage();
    if (message != null) {
      if (!calendarSmeEngine.processMessage(message) && !secretRequestProcessor.processMessage(message)) {
        Log.error("Unknown message format!");
        Log.error("Message from abonent: " + message.getSourceAddress() + "; to abonent: " + message.getDestinationAddress() + "; message: " + message.getMessageString());
        sendResponse(message, Data.ESME_RX_P_APPN);
      } 
    }
  }

  private void sendResponse(Message msg, int status) {
    try {
      msg.setStatus(status);
      multiplexor.sendResponse(msg);
      Log.info(NAME + ": Delivery response sent, address #" + msg.getDestinationAddress() + "; abonent #" + msg.getSourceAddress() + "; status #" + msg.getStatus());
    } catch (SMPPException e) {
      Log.warn("Exception occured sending delivery response.", e);
    }
  }
}