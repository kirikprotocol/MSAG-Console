package ru.sibinco.otasme.engine;

import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.SMPPException;
import ru.sibinco.otasme.Sme;
import ru.sibinco.otasme.SmeProperties;
import ru.sibinco.otasme.engine.smscenters.SMSCenters;
import ru.sibinco.otasme.network.OutgoingObject;
import ru.sibinco.otasme.utils.Service;
import com.logica.smpp.Data;

/**
 * User: artem
 * Date: 26.10.2006
 */

public final class SmeEngine extends Service {

  private static final Category log = Category.getInstance(SmeEngine.class);

  public SmeEngine() {
    super(log);
  }

  public void iterativeWork() {
    final Message incomingMessage = Sme.inQueue.getIncomingObject().getMessage();

     final String abonentAddr = (incomingMessage.getType() == Message.TYPE_WTS_REQUEST) ? incomingMessage.getWtsRequestReference() :
                                                                                             incomingMessage.getSourceAddress();
//    final String smeAddr = incomingMessage.getDestinationAddress();
    final String smeAddr = SmeProperties.SmeEngine.SME_ADDRESS;

    try {
      Session session = SessionsRegistry.getSession(getSessionId(incomingMessage));

      if (session == null) {
        if (incomingMessage.getType() != Message.TYPE_DELIVER && incomingMessage.getType() != Message.TYPE_WTS_REQUEST) {
          sendResponse(incomingMessage, Data.ESME_ROK);
          log.error("Unexpected message with type=" + incomingMessage.getType());
          return;
        }

        String smscenterNumber = SMSCenters.getSMSCenterNumberByAbonent(abonentAddr);
        if (smscenterNumber == null) {
          log.info("WARNING!!! Can't find macro region for abonent " + abonentAddr);
          sendResponse(incomingMessage, Data.ESME_ROK);
          sendMessage(SmeProperties.SmeEngine.NUMBER_NOT_FOUND_ERROR_TEXT, smeAddr, abonentAddr, incomingMessage.getConnectionName());
          return;
        }

        session = new Session(abonentAddr, smeAddr, smscenterNumber);
      }

      session.processMessage(incomingMessage);
    } catch (SessionsRegistry.SessionsRegistryException e) {
      log.error("Can't start new session", e);
      sendResponse(incomingMessage, Data.ESME_ROK);
      sendMessage(SmeProperties.SmeEngine.SESSIONS_REGISTRY_OVERFLOW_TEXT, smeAddr, abonentAddr, incomingMessage.getConnectionName());
    } catch (SessionsRegistry.SessionsRegistryOverflowException e) {
      log.error("Can't start new session", e);
      sendResponse(incomingMessage, Data.ESME_ROK);
      sendMessage(SmeProperties.SmeEngine.SESSIONS_REGISTRY_OVERFLOW_TEXT, smeAddr, abonentAddr, incomingMessage.getConnectionName());
    } catch (Session.UnexpectedMessageException e) {
      sendResponse(incomingMessage, Data.ESME_ROK);
      log.error("Unexpected message", e);
    } catch (Throwable e) {
      sendResponse(incomingMessage, Data.ESME_RSYSERR);
      log.error(e);
    }
  }

  private static String getSessionId(Message incomingMessage) {
    if (incomingMessage.getType() == Message.TYPE_WTS_REQUEST)
      return incomingMessage.getWtsRequestReference();
    return incomingMessage.getSourceAddress();
  }

  private static void sendMessage(String msg, String sourceAddr, String destAddr, String connectionName) {
    final Message message = new Message();
    message.setSourceAddress(sourceAddr);
    message.setDestinationAddress(destAddr);
    message.setMessageString(msg);
    message.setConnectionName(connectionName);
    Sme.outQueue.addOutgoingObject(new OutgoingObject(message));
  }

  private static void sendResponse(Message msg, int status) {
    try {
      msg.setStatus(status);
      Sme.multiplexor.sendResponse(msg);
      log.debug("Delivery response sent, address #" + msg.getDestinationAddress() + "; abonent #" + msg.getSourceAddress() + "; status #" + msg.getStatus());
    } catch (SMPPException e) {
      log.warn("Exception occured sending delivery response.", e);
    }
  }
}
