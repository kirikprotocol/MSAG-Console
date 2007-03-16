package ru.sibinco.smsx.engine;

import com.logica.smpp.Data;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.Multiplexor;
import ru.aurorisoft.smpp.SMPPException;
import ru.sibinco.smsx.InitializationException;
import ru.sibinco.smsx.SmeProperties;
import ru.sibinco.smsx.network.IncomingQueue;
import ru.sibinco.smsx.network.OutgoingObject;
import ru.sibinco.smsx.network.OutgoingQueue;
import ru.sibinco.smsx.services.InitializationInfo;
import ru.sibinco.smsx.services.InternalService;
import ru.sibinco.smsx.services.ProcessException;
import ru.sibinco.smsx.services.calendar.CalendarService;
import ru.sibinco.smsx.services.redirector.RedirectorService;
import ru.sibinco.smsx.services.secret.SecretService;
import ru.sibinco.smsx.services.sponsored.SponsoredService;
import ru.sibinco.smsx.services.sponsored.SponsoredServiceNew;
import ru.sibinco.smsx.utils.Service;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

/**
 * User: artem
 * Date: Jul 27, 2006
 */

public final class SmeEngine extends Service {
  private static final org.apache.log4j.Category log = org.apache.log4j.Category.getInstance(SmeEngine.class);

  private final IncomingQueue inQueue;
  private final OutgoingQueue outQueue;
  private final Multiplexor multiplexor;

  private final List services = new ArrayList();

  public SmeEngine(final IncomingQueue inQueue, final OutgoingQueue outQueue, final Multiplexor multiplexor) {
    super(log);
    if (inQueue == null)
      throw new InitializationException("Incoming queue is not specified");
    if (outQueue == null)
      throw new InitializationException("Outgoing queue is not specified");
    if (multiplexor == null)
      throw new InitializationException("Multiplexor is not specified");

    this.inQueue = inQueue;
    this.outQueue = outQueue;
    this.multiplexor = multiplexor;

    final RedirectorService redirectorService = new RedirectorService();
    redirectorService.init(new InitializationInfo(multiplexor, outQueue, SmeProperties.REDIRECTOR_ADDRESS));

    final CalendarService calendarService = new CalendarService();
    calendarService.init(new InitializationInfo(multiplexor, outQueue, SmeProperties.CALENDAR_ADDRESS));

    final SecretService secretService = new SecretService();
    secretService.init(new InitializationInfo(multiplexor, outQueue, SmeProperties.SECRET_ADDRESS));

    final SponsoredServiceNew sponsoredService = new SponsoredServiceNew();
    sponsoredService.init(new InitializationInfo(multiplexor, outQueue, SmeProperties.SPONSORED_ADDRESS));

    // Order is important here
    services.add(redirectorService);
    services.add(calendarService);
    services.add(secretService);
    services.add(sponsoredService);

    log.info("Initialization ok");
  }

  public void iterativeWork() {
    try {
      final Message message = inQueue.getIncomingObject().getMessage();
      if (message != null) {
        try {
          for (Iterator iter = services.iterator(); iter.hasNext();) {
            final InternalService service = ((InternalService)iter.next());
            if (service.processMessage(message)) {
              log.info("Message had been processed by service: " + service.getName());
              return;
            }
          }
        } catch (ProcessException e) {
          log.error(e);
          sendResponse(message, Data.ESME_RSYSERR);
          return;
        }

        log.error("Unknown message format!");
        log.error("Message from abonent: " + message.getSourceAddress() + "; to abonent: " + message.getDestinationAddress() + "; message: " + message.getMessageString());
        sendResponse(message, Data.ESME_RX_P_APPN);

        final String rule = SmeProperties.getRuleMessage(message.getDestinationAddress());
        if (rule != null) {
          log.info("Send rule message: " + rule);
          final Message msg = new Message();
          msg.setSourceAddress(message.getDestinationAddress());
          msg.setDestinationAddress(message.getSourceAddress());
          msg.setMessageString(rule);
          outQueue.addOutgoingObject(new OutgoingObject(msg));
        }
      }
    } catch (Throwable e) {
      log.error(e);
    }
  }

  private void sendResponse(Message msg, int status) {
    try {
      msg.setStatus(status);
      multiplexor.sendResponse(msg);
      log.info("Delivery response sent, address #" + msg.getDestinationAddress() + "; abonent #" + msg.getSourceAddress() + "; status #" + msg.getStatus());
    } catch (SMPPException e) {
      log.warn("Exception occured sending delivery response.", e);
    }
  }
}