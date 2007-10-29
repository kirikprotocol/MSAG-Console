package ru.sibinco.otasme.engine.service;

import com.eyeline.sme.utils.statemachine.EventSink;
import ru.sibinco.otasme.SmeProperties;
import ru.sibinco.otasme.network.smpp.SMPPOutgoingQueue;
import ru.sibinco.smsc.utils.smscenters.SmsCenter;
import ru.sibinco.smsc.utils.smscenters.SmsCentersList;
import snaq.db.ConnectionPool;

/**
 * User: artem
 * Date: Oct 5, 2007
 */

class UnsubscribeAbonentState extends AbstractSubscriptionState {

  public UnsubscribeAbonentState(EventSink eventSink, ConnectionPool pool, SMPPOutgoingQueue outQueue, SmsCentersList smsCenters) {
    super(eventSink, pool, outQueue, smsCenters);
  }

  public int unsubscribeExternalAbonent(String abonentAddress) {
    try {
      if (!abonentPresentsInDB(abonentAddress)) { // For unsubscribed abonents

        // Check is SMSC for abonent exists in config
        if (getSmscAddressForAbonent(abonentAddress) == null)
          return STATUS_SMSC_NOT_FOUND;

        // Set next state for abonent
        getEventSink().castEvent(new StateEvent(StateEvent.TYPE_SET_WTS_SUBSCRIPTION_STATE, abonentAddress));
        // Send command to OTA
        sendSRCommand(SmeProperties.Session.SMSEXTRA_NUMBER, abonentAddress);

        return STATUS_ACCEPTED;
      }
      // For subscribed abonents

      // Remove abonent from DB
      removeAbonentFromDB(abonentAddress);

      return STATUS_ABONENT_UNSUBSCRIBED;

    } catch (Throwable e) {
      log.error("Unsubscription error for " + abonentAddress,e);
      getEventSink().castEvent(new StateEvent(StateEvent.TYPE_REMOVE_STATE, abonentAddress));
      return STATUS_SYSTEM_ERROR;
    }
  }

  public int unsubscribeInternalAbonent(String abonentAddress) {
    try {
      // Check is SMSC for abonent exists in config
      final SmsCenter smsCenter = getSmscAddressForAbonent(abonentAddress);
      if (smsCenter == null)
        return STATUS_SMSC_NOT_FOUND;

      // Set next state for abonent
      getEventSink().castEvent(new StateEvent(StateEvent.TYPE_SET_WTS_UNSUBSCRIPTION_STATE, abonentAddress));
      // Send command to OTA
      sendSRCommand(smsCenter.getAddress(), abonentAddress);

      return STATUS_ACCEPTED;

    } catch (Throwable e) {
      log.error("Unsubscription error for " + abonentAddress,e);
      getEventSink().castEvent(new StateEvent(StateEvent.TYPE_REMOVE_STATE, abonentAddress));
      return STATUS_SYSTEM_ERROR;
    }
  }

  public int subscribeAbonent(String abonentAddress, boolean isAbonentExternal) {
    log.error("State machine in unsubscription mode, but subscription cmd received");
    return STATUS_UNEXPECTED_MESSAGE;
  }

  public int unsubscribeAbonent(String abonentAddress, boolean isAbonentExternal) {
    return (isAbonentExternal) ? unsubscribeExternalAbonent(abonentAddress) : unsubscribeInternalAbonent(abonentAddress);
  }

  public int handleWTSResponse(String abonentAddress, long code, int status) {
    log.error("State machine in unsubscription mode, but handle WTS response cmd received");
    return STATUS_UNEXPECTED_MESSAGE;
  }

  public int handleWTSSendError(String abonentAddress) {
    log.error("State machine in unsubscription mode, but handle WTS send error cmd received");
    return STATUS_SYSTEM_ERROR;
  }

  protected void doBeforeRemove(String abonentAddress) {
  }
}
