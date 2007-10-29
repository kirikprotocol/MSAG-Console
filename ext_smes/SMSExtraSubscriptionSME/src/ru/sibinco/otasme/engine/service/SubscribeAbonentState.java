package ru.sibinco.otasme.engine.service;

import com.eyeline.sme.utils.statemachine.EventSink;
import ru.sibinco.otasme.SmeProperties;
import ru.sibinco.otasme.network.smpp.SMPPOutgoingQueue;
import ru.sibinco.smsc.utils.smscenters.SmsCentersList;
import snaq.db.ConnectionPool;

/**
 * User: artem
 * Date: Oct 5, 2007
 */

class SubscribeAbonentState extends AbstractSubscriptionState {

  public SubscribeAbonentState(EventSink eventSink, ConnectionPool pool, SMPPOutgoingQueue outQueue, SmsCentersList smsCenters) {
    super(eventSink, pool, outQueue, smsCenters);
  }

  private int subscribeExternalAbonent(String abonentAddress) {
    try {
      // Check is SMSC for abonent exists in config
      if (getSmscAddressForAbonent(abonentAddress) == null)
        return STATUS_SMSC_NOT_FOUND;

      // Set next state for abonent
      getEventSink().castEvent(new StateEvent(StateEvent.TYPE_SET_WTS_SUBSCRIPTION_STATE, abonentAddress));
      // Send command to OTA
      sendSRCommand(SmeProperties.Session.SMSEXTRA_NUMBER, abonentAddress);

      return STATUS_ACCEPTED;
    } catch (Throwable e) {
      log.error("Subscription error for " + abonentAddress,e);
      getEventSink().castEvent(new StateEvent(StateEvent.TYPE_REMOVE_STATE, abonentAddress));
      return STATUS_SYSTEM_ERROR;
    }
  }

  private int subscribeInternalAbonent(String abonentAddress) {
    try {
      // Store abonent address in DB
      addAbonentIntoDB(abonentAddress);

      return STATUS_ABONENT_SUBSCRIBED;
    } catch (Throwable e) {
      log.error("Subscription error for " + abonentAddress,e);
      return STATUS_SYSTEM_ERROR;
    }
  }

  public int subscribeAbonent(String abonentAddress, boolean isAbonentExternal) {
    return (isAbonentExternal) ? subscribeExternalAbonent(abonentAddress) : subscribeInternalAbonent(abonentAddress);
  }

  public int unsubscribeAbonent(String abonentAddress, boolean isAbonentExternal) {
    log.error("State machine in subscription mode, but unsubscribe cmd received");
    return STATUS_UNEXPECTED_MESSAGE;
  }

  public int handleWTSResponse(String abonentAddress, long code, int status) {
    log.error("State machine in subscription mode, but handle WTS response cmd received");
    return STATUS_UNEXPECTED_MESSAGE;
  }

  public int handleWTSSendError(String abonentAddress) {
    log.error("State machine in subscription mode, but handle WTS send error cmd received");
    return STATUS_SYSTEM_ERROR;
  }

  protected void doBeforeRemove(String abonentAddress) {
  }
}
