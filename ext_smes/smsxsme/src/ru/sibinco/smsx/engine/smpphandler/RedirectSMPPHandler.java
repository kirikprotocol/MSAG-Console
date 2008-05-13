package ru.sibinco.smsx.engine.smpphandler;

import com.logica.smpp.Data;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.PDU;
import ru.sibinco.smsx.engine.smpphandler.redirects.Redirect;
import ru.sibinco.smsx.engine.smpphandler.redirects.RedirectsList;
import ru.sibinco.smsx.network.smppnetwork.SMPPMultiplexor;
import ru.sibinco.smsx.network.smppnetwork.SMPPTransportObject;

import java.io.File;
import java.util.regex.Matcher;

/**
 * User: artem
 * Date: 03.07.2007
 */

class RedirectSMPPHandler extends SMPPHandler {
  private static final Category log = Category.getInstance("REDIRECT SMPP");

  public static int DELIVER_OK = 0;
  public static int DELIVER_TMP_ERROR = 1;
  public static int DELIVER_THROTTLED_ERROR = 2;
  public static int DELIVER_PERM_ERROR = 3;
  public static int DELIVER_UNKNOWN_ERROR = 4;

  private final RedirectsList redirectsList;

  RedirectSMPPHandler(String configDir, SMPPMultiplexor multiplexor) {
    super(multiplexor);
    redirectsList = new RedirectsList(new File(configDir, "smpphandlers/redirects.xml").getAbsolutePath());
  }

  protected boolean handleInObj(SMPPTransportObject inObj) {
    final long start = System.currentTimeMillis();

    try {
      if (inObj.getIncomingMessage() != null && inObj.getIncomingMessage().getMessageString() != null) {
        final Message message = inObj.getIncomingMessage();
        final String msg = inObj.getIncomingMessage().getMessageString().trim();

        if (log.isInfoEnabled())
          log.info("Msg srcaddr=" + message.getSourceAddress() + "; dstaddr=" + message.getDestinationAddress());

        final Redirect redirect = redirectsList.getRedirectByMessage(msg);
        if (redirect != null) {
          final Matcher matcher = redirect.getPrefix().matcher(msg);
          matcher.find();
          message.setMessageString(redirect.getNewPrefix() + message.getMessageString().substring(matcher.end()));
          if (log.isInfoEnabled())
            log.info("Redirect dstaddr=" + redirect.getAddress() + "; msg=" + message.getMessageString());
          message.setDestinationAddress(redirect.getAddress());
          sendMessage(new RedirectorOutgoingObject(message));

          return true;
        }
      }

      return false;

    } catch (Throwable e) {
      log.error(e,e);
      sendResponse(inObj.getIncomingMessage(), Data.ESME_RSYSERR);
      return true;

    } finally {
      if (log.isInfoEnabled())
        log.info("Time=" + (System.currentTimeMillis() - start));
    }
  }

  public class RedirectorOutgoingObject extends SMPPTransportObject {

    private final int sequenceNumber;

    public RedirectorOutgoingObject(Message outgoingMessage) {
      super();
      this.sequenceNumber = outgoingMessage.getSequenceNumber();
      setOutgoingMessage(outgoingMessage);
    }

    public void handleResponse(PDU response) {
      final Message outMsg = getOutgoingMessage();
      outMsg.setSequenceNumber(sequenceNumber);

      sendResponse(outMsg, response.getStatus());
    }

  }

}
