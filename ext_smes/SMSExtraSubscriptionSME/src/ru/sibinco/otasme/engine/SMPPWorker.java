package ru.sibinco.otasme.engine;

import com.eyeline.sme.utils.worker.IterativeWorker;
import com.logica.smpp.Data;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.sibinco.otasme.SmeProperties;
import ru.sibinco.otasme.engine.service.SubscriptionStateMachine;
import ru.sibinco.otasme.network.smpp.SMPPIncomingQueue;
import ru.sibinco.otasme.network.smpp.SMPPMultiplexor;
import ru.sibinco.otasme.network.smpp.SMPPOutgoingQueue;
import ru.sibinco.otasme.network.smpp.SMPPTransportObject;

import java.util.regex.Pattern;

/**
 * User: artem
 * Date: Oct 8, 2007
 */

public class SMPPWorker extends IterativeWorker {

  private static final Category log = Category.getInstance("SMPP HANDLER");

  private static final Pattern OFF = Pattern.compile("(O|o)(F|f)(F|f)");

  private final SMPPIncomingQueue inQueue;
  private final SMPPOutgoingQueue outQueue;
  private final SMPPMultiplexor multiplexor;
  private final SubscriptionStateMachine stateMachine;

  public SMPPWorker(SMPPMultiplexor multiplexor, SubscriptionStateMachine stateMachine) {
    super(log);
    this.multiplexor = multiplexor;
    this.inQueue = multiplexor.getInQueue();
    this.outQueue = multiplexor.getOutQueue();
    this.stateMachine = stateMachine;
  }

  protected void iterativeWork() {
    final SMPPTransportObject inObj = inQueue.getInObj();
    if (!isStarted())
      return;

    if (inObj.getStatus() != SMPPTransportObject.STATUS_CANT_SEND) { // For usual incoming messages

      final Message incomingMessage = inObj.getIncomingMessage();

      // Retrive abonent address
      final String abonentAddr = (incomingMessage.getType() == Message.TYPE_WTS_REQUEST) ? incomingMessage.getWtsRequestReference() :
                                                                                             incomingMessage.getSourceAddress();

      log.info("Msg: srcaddr=" + abonentAddr + "; connector=" + incomingMessage.getConnectionName() + "; wts=" + (incomingMessage.getType() == Message.TYPE_WTS_REQUEST));

      multiplexor.sendResponse(incomingMessage, Data.ESME_ROK);

      final boolean isAbonentExternal = incomingMessage.getConnectionName().equalsIgnoreCase("mar");

      if (incomingMessage.getType() != Message.TYPE_WTS_REQUEST ) { // For messages from abonents
        final String msg = incomingMessage.getMessageString();
        if (msg != null && OFF.matcher(msg).matches())
          handleUnregisterRequest(abonentAddr, isAbonentExternal, incomingMessage.getConnectionName());
        else
          handleRegisterRequest(abonentAddr, isAbonentExternal, incomingMessage.getConnectionName());

      } else // For messages from OTA
        handleWTSRequest(abonentAddr, incomingMessage.getWTSErrorCode(), incomingMessage.getWtsErrorStatus(), incomingMessage.getConnectionName());

    } else { // For error responses

      final Message outgoingMessage = inObj.getOutgoingMessage();

      final String abonentAddr = (outgoingMessage.getType() == Message.TYPE_WTS_REQUEST) ? outgoingMessage.getWtsRequestReference() :
                                                                                             outgoingMessage.getSourceAddress();
      if (outgoingMessage.getType() == Message.TYPE_WTS_REQUEST)
        handleWTSSendError(abonentAddr, outgoingMessage.getConnectionName());
    }
  }

  private void handleUnregisterRequest(String abonentAddress, boolean isAbonentExternal, String connectionName) {
    final int status = stateMachine.unsubscribeAbonent(abonentAddress, isAbonentExternal);
    switch (status) {
      case SubscriptionStateMachine.STATUS_ACCEPTED:
        log.info("Unsubscription request for " + abonentAddress + " was accepted");
        break;
      case SubscriptionStateMachine.STATUS_ABONENT_UNSUBSCRIBED:
        log.info("Abonent sucessfully unsubscribed");
        if (isAbonentExternal)
          sendMessage(SmeProperties.SmeEngine.SME_ADDRESS, abonentAddress, SmeProperties.Session.MAR_OFF_TEXT, connectionName);
        break;
      case SubscriptionStateMachine.STATUS_UNEXPECTED_MESSAGE:
        log.info("Unexpected message appeared for " + abonentAddress);
        sendMessage(SmeProperties.SmeEngine.SME_ADDRESS, abonentAddress, SmeProperties.Session.UNEXPECTED_MESSAGE_ERROR_TEXT, connectionName);
        break;
      case SubscriptionStateMachine.STATUS_SYSTEM_ERROR:
        log.info("System error appeared for " + abonentAddress);
        sendMessage(SmeProperties.SmeEngine.SME_ADDRESS, abonentAddress, SmeProperties.Session.INTERNAL_ERROR, connectionName);
        break;
      case SubscriptionStateMachine.STATUS_SMSC_NOT_FOUND:
        log.info("SMSC address not found for " + abonentAddress);
        sendMessage(SmeProperties.SmeEngine.SME_ADDRESS, abonentAddress, SmeProperties.SmeEngine.NUMBER_NOT_FOUND_ERROR_TEXT, connectionName);
        break;
      default:
        log.error("Unexpected status from unregister cmd = " + status);
    }
  }

  private void handleRegisterRequest(String abonentAddress, boolean isAbonentExternal, String connectionName) {
    final int status = stateMachine.subscribeAbonent(abonentAddress, isAbonentExternal);
    switch (status) {
      case SubscriptionStateMachine.STATUS_ACCEPTED:
        log.info("Subscription request for " + abonentAddress + " was accepted");
        break;
      case SubscriptionStateMachine.STATUS_ABONENT_SUBSCRIBED:
        log.info("Abonent sucessfully subscribed");
        if (!isAbonentExternal)
          sendMessage(SmeProperties.SmeEngine.SME_ADDRESS, abonentAddress, SmeProperties.Session.SMSC_ON_TEXT, connectionName);
        break;
      case SubscriptionStateMachine.STATUS_UNEXPECTED_MESSAGE:
        log.info("Unexpected message appeared for " + abonentAddress);
        sendMessage(SmeProperties.SmeEngine.SME_ADDRESS, abonentAddress, SmeProperties.Session.UNEXPECTED_MESSAGE_ERROR_TEXT, connectionName);
        break;
      case SubscriptionStateMachine.STATUS_SYSTEM_ERROR:
        log.info("System error appeared for " + abonentAddress);
        sendMessage(SmeProperties.SmeEngine.SME_ADDRESS, abonentAddress, SmeProperties.Session.INTERNAL_ERROR, connectionName);
        break;
      case SubscriptionStateMachine.STATUS_SMSC_NOT_FOUND:
        log.info("SMSC address not found for " + abonentAddress);
        sendMessage(SmeProperties.SmeEngine.SME_ADDRESS, abonentAddress, SmeProperties.SmeEngine.NUMBER_NOT_FOUND_ERROR_TEXT, connectionName);
        break;
      default:
        log.error("Unexpected status from register cmd = " + status);
    }
  }

  private void handleWTSRequest(String abonentAddress, long code, int stat, String connectionName) {
    final int status = stateMachine.handleWTSResponse(abonentAddress, code, stat);
    switch (status) {
      case SubscriptionStateMachine.STATUS_ACCEPTED:
        log.info("Subscription request for " + abonentAddress + " was accepted");
        break;
      case SubscriptionStateMachine.STATUS_ABONENT_SUBSCRIBED:
        log.info("Abonent sucessfully subscribed");
        break;
      case SubscriptionStateMachine.STATUS_ABONENT_UNSUBSCRIBED:
        log.info("Abonent sucessfully unsubscribed");
        break;
      case SubscriptionStateMachine.STATUS_WTS_SUBSCRIPTION_ERROR:
        log.info("WTS error appeared during subscription for " + abonentAddress);
        sendMessage(SmeProperties.SmeEngine.SME_ADDRESS, abonentAddress, SmeProperties.Session.ON_ERROR_TEXT, connectionName);
        break;
      case SubscriptionStateMachine.STATUS_WTS_UNSUBSCRIPTION_ERROR:
        log.info("WTS error appeared during unsubscription for " + abonentAddress);
        sendMessage(SmeProperties.SmeEngine.SME_ADDRESS, abonentAddress, SmeProperties.Session.OFF_ERROR_TEXT, connectionName);
        break;
      case SubscriptionStateMachine.STATUS_UNEXPECTED_MESSAGE:
        log.info("Unexpected message appeared for " + abonentAddress);
        sendMessage(SmeProperties.SmeEngine.SME_ADDRESS, abonentAddress, SmeProperties.Session.UNEXPECTED_MESSAGE_ERROR_TEXT, connectionName);
        break;
      case SubscriptionStateMachine.STATUS_SYSTEM_ERROR:
        log.info("System error appeared for " + abonentAddress);
        sendMessage(SmeProperties.SmeEngine.SME_ADDRESS, abonentAddress, SmeProperties.Session.INTERNAL_ERROR, connectionName);
        break;
      default:
        log.error("Unexpected status from wts cmd = " + status);
    }
  }

  private void handleWTSSendError(String abonentAddress, String connectionName) {
    log.error("Can't send WTS request for " + abonentAddress);
    stateMachine.handleWTSSendError(abonentAddress);
    sendMessage(SmeProperties.SmeEngine.SME_ADDRESS, abonentAddress, SmeProperties.Session.INTERNAL_ERROR, connectionName);
  }

  private void sendMessage(String sourceAddress, String destinationAddress, String message, String connectionName) {
    final Message msg = new Message();
    msg.setSourceAddress(sourceAddress);
    msg.setDestinationAddress(destinationAddress);
    msg.setMessageString(message);
    msg.setConnectionName("smsx"); // TODO

    final SMPPTransportObject outObj = new SMPPTransportObject();
    outObj.setOutgoingMessage(msg);
    outQueue.addOutgoingObject(outObj);
  }

  protected void stopCurrentWork() {
    inQueue.notifyWaiters();
  }
}
