package ru.sibinco.otasme.engine;

import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.sibinco.otasme.Sme;
import ru.sibinco.otasme.engine.template.MacroRegion;
import ru.sibinco.otasme.engine.template.Templates;
import ru.sibinco.otasme.network.OutgoingObject;
import ru.sibinco.otasme.utils.Service;
import ru.sibinco.otasme.utils.Utils;

import java.util.Properties;

/**
 * User: artem
 * Date: 26.10.2006
 */

public final class SmeEngine extends Service {

  private static final Category log = Category.getInstance(SmeEngine.class);

  private static final String NUMBER_NOT_FOUND_ERROR_TEXT;
  private static final String SESSIONS_REGISTRY_OVERFLOW_TEXT;

  static {
    final Properties config = Utils.loadConfig("sme.properties");
    NUMBER_NOT_FOUND_ERROR_TEXT = Utils.loadString(config, "sme.engine.number.not.found.error.text");
    SESSIONS_REGISTRY_OVERFLOW_TEXT = Utils.loadString(config, "sme.engine.sessions.registry.full.text");
  }


  public SmeEngine() {
    super(log);
  }

  public void iterativeWork() {
    final Message incomingMessage = Sme.inQueue.getIncomingObject().getMessage();

     final String abonentAddr = (incomingMessage.getType() == Message.TYPE_WTS_REQUEST) ? incomingMessage.getWtsRequestReference() :
                                                                                             incomingMessage.getSourceAddress();
    final String smeAddr = incomingMessage.getDestinationAddress();

    try {
      Session session = SessionsRegistry.getSession(getSessionId(incomingMessage));

      if (session == null) {
        if (incomingMessage.getType() != Message.TYPE_DELIVER && incomingMessage.getType() != Message.TYPE_WTS_REQUEST) {
          log.error("Unexpected message with type=" + incomingMessage.getType());
          return;
        }

        MacroRegion macroRegion = Templates.getMacroRegionByAbonentNumber(abonentAddr);
        if (macroRegion == null) {
          log.info("WARNING!!! Can't find macro region for abonent " + abonentAddr);
          sendMessage(NUMBER_NOT_FOUND_ERROR_TEXT, smeAddr, abonentAddr);
          return;
        }

        session = new Session(abonentAddr, smeAddr, macroRegion);
      }

      session.processMessage(incomingMessage);
    } catch (SessionsRegistry.SessionsRegistryException e) {
      log.error("Can't start new session", e);
      sendMessage(SESSIONS_REGISTRY_OVERFLOW_TEXT, smeAddr, abonentAddr);
    } catch (SessionsRegistry.SessionsRegistryOverflowException e) {
      log.error("Can't start new session", e);
      sendMessage(SESSIONS_REGISTRY_OVERFLOW_TEXT, smeAddr, abonentAddr);
    } catch (Session.UnexpectedMessageException e) {
      log.error("Unexpected message", e);
    }
  }

  private static String getSessionId(Message incomingMessage) {
    if (incomingMessage.getType() == Message.TYPE_WTS_REQUEST)
      return incomingMessage.getWtsRequestReference();
    return incomingMessage.getSourceAddress();
  }

  private static void sendMessage(String msg, String sourceAddr, String destAddr) {
    final Message message = new Message();
    message.setSourceAddress(sourceAddr);
    message.setDestinationAddress(destAddr);
    message.setMessageString(msg);
    Sme.outQueue.addOutgoingObject(new OutgoingObject(message));
  }
}
