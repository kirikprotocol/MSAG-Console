package ru.sibinco.smsx.services.redirector;

import com.logica.smpp.Data;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.Multiplexor;
import ru.sibinco.smsx.network.OutgoingQueue;
import ru.sibinco.smsx.services.MessageRedirector;
import ru.sibinco.smsx.services.ServiceProcessor;
import ru.sibinco.smsx.utils.BlockingQueue;

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
    try {
      log.info("=====================================================================================");
      log.info("Processing message: from abonent = " + message.getSourceAddress() + ", to abonent = " + message.getDestinationAddress() + ", message = " + message.getMessageString());

      log.info("Redirect to address " + parsedMessage.getToAddress());
      redirectMessage(message, parsedMessage.getToAddress());
    } catch (Exception e) {
      log.error(e);
      sendResponse(message, Data.ESME_RSYSERR);
    }
    return true;
  }

  private void redirectMessage(final Message message, final String toAddress) {
    MessageRedirector.redirectMessage(message, toAddress, outQueue, multiplexor);
  }
}
