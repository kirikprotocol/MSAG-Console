package ru.sibinco.smsx.services.redirector;

import com.logica.smpp.Data;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.Multiplexor;
import ru.sibinco.smsx.network.OutgoingQueue;
import ru.sibinco.smsx.services.ServiceProcessor;
import ru.sibinco.smsx.services.RedirectorOutgoingObject;
import ru.sibinco.smsx.services.calendar.CalendarService;
import ru.sibinco.smsx.services.redirector.redirects.Redirect;
import ru.sibinco.smsx.utils.BlockingQueue;

import java.util.regex.Matcher;

/**
 * User: artem
 * Date: 24.01.2007
 */
final class RedirectorRequestProcessor extends ServiceProcessor {

  private static final org.apache.log4j.Category log = org.apache.log4j.Category.getInstance(RedirectorRequestProcessor.class);

  private final BlockingQueue inQueue;

  public RedirectorRequestProcessor(BlockingQueue inQueue, OutgoingQueue outQueue, Multiplexor multiplexor) {
    super(log, outQueue, multiplexor);
    this.inQueue = inQueue;
  }

  public void iterativeWork() {
    try {
      processMessage((ParsedMessage)inQueue.getObject());
    } catch (Throwable e) {
      log.error("Error", e);
    }
  }

  public boolean processMessage(final ParsedMessage parsedMessage) {
    final Message message = parsedMessage.getMessage();
    final Redirect redirect = parsedMessage.getRedirect();
    try {
      log.info("=====================================================================================");
      log.info("Processing message: from abonent = " + message.getSourceAddress() + ", to abonent = " + message.getDestinationAddress() + ", message = " + message.getMessageString());

      final Matcher matcher = redirect.getPrefix().matcher(message.getMessageString());
      matcher.find();
      message.setMessageString(redirect.getNewPrefix() + message.getMessageString().substring(matcher.end()));

      log.info("Redirect to address " + redirect.getAddress());
      message.setDestinationAddress(redirect.getAddress());
      
      outQueue.addOutgoingObject(new RedirectorOutgoingObject(message, multiplexor));

    } catch (Exception e) {
      log.error(e);
      sendResponse(message, Data.ESME_RSYSERR);
    }
    return true;
  }

}
