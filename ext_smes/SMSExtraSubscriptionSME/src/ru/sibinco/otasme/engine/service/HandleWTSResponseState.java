package ru.sibinco.otasme.engine.service;

import com.eyeline.sme.utils.statemachine.EventSink;
import ru.aurorisoft.smpp.Message;
import ru.sibinco.otasme.SmeProperties;
import ru.sibinco.otasme.network.smpp.SMPPOutgoingQueue;
import ru.sibinco.smsc.utils.smscenters.SmsCentersList;
import snaq.db.ConnectionPool;

import java.util.HashMap;
import java.util.Map;

/**
 * User: artem
 * Date: Oct 5, 2007
 */

class HandleWTSResponseState extends AbstractSubscriptionState {

  private final boolean enableService;
  private final Map abonentsRepeatsCounts = new HashMap();

  public HandleWTSResponseState(EventSink eventSink, ConnectionPool pool, SMPPOutgoingQueue outQueue, SmsCentersList smsCenters, boolean enableService) {
    super(eventSink, pool, outQueue, smsCenters);
    this.enableService = enableService;
  }

  public int subscribeAbonent(String abonentAddress, boolean isAbonentExternal) {
    log.error("State machine in WTS mode, but subscription cmd received");
    return STATUS_UNEXPECTED_MESSAGE;
  }

  public int unsubscribeAbonent(String abonentAddress, boolean isAbonentExternal) {
    log.error("State machine in WTS mode, but unsubscription cmd received");
    return STATUS_UNEXPECTED_MESSAGE;
  }

  public int handleWTSResponse(String abonentAddress, long code, int status) {
    try {
      // Remove repeats count for abonent
      Integer repeatsCount = (Integer)abonentsRepeatsCounts.remove(abonentAddress);

      // TODO Temporary simulate OTA error!!!!!!!!!
      code = 1;
      status = Message.WTS_ERROR_STATUS_PERM;

      if (code != 0) { // Error occured

        log.info("OTA Platform return error with code = " + code + " for " + abonentAddress);

        if (status != Message.WTS_ERROR_STATUS_PERM) { // For temporary error

          log.info("It is temporary error. Repeat SR_COMMAND after " + SmeProperties.CommandsRepeater.RETRY_PERIOD + " ms");

          // Calculate repeats count for abonent
          int repeats = (repeatsCount == null) ? 0 : repeatsCount.intValue() + 1;

          if (repeats < SmeProperties.Session.MAX_OTA_MESSAGE_REPEATS) {
            // Store repeats count for abonent
            abonentsRepeatsCounts.put(abonentAddress, new Integer(repeats));
            // Set next state for abonent
            getEventSink().castEvent(new StateEvent(enableService ? StateEvent.TYPE_SET_WTS_SUBSCRIPTION_STATE : StateEvent.TYPE_SET_WTS_UNSUBSCRIPTION_STATE, abonentAddress));
            // Send command to OTA after some delay (RETRY_PERIOD)
            sendSRCommand(SmeProperties.Session.SMSEXTRA_NUMBER, abonentAddress, SmeProperties.CommandsRepeater.RETRY_PERIOD);
            return STATUS_ACCEPTED;
          }

          log.info("Max retries for abonent " + abonentAddress);
        }

        // For permanent error or max retries in temporary error
        return enableService ? STATUS_WTS_SUBSCRIPTION_ERROR : STATUS_WTS_UNSUBSCRIPTION_ERROR;

      } else { // No error occured

        log.info("OTA Platform return success");

        // Store or remove abonent from DB
        if (enableService)
          addAbonentIntoDB(abonentAddress);
        else
          removeAbonentFromDB(abonentAddress);

        return enableService ? STATUS_ABONENT_SUBSCRIBED : STATUS_ABONENT_UNSUBSCRIBED;
      }

    } catch (Throwable e) {
      log.error("Handle WTS response error for " + abonentAddress, e);
      abonentsRepeatsCounts.remove(abonentAddress);
      return STATUS_SYSTEM_ERROR;
    }
  }

  public int handleWTSSendError(String abonentAddress) {
    log.error("WTS Send error for " + abonentAddress);
    abonentsRepeatsCounts.remove(abonentAddress);
    return STATUS_SYSTEM_ERROR;
  }

  protected void doBeforeRemove(String abonentAddress) {
    abonentsRepeatsCounts.remove(abonentAddress);
  }
}
