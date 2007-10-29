package ru.sibinco.otasme.engine.service;

import com.eyeline.sme.utils.statemachine.State;

/**
 * User: artem
 * Date: Oct 5, 2007
 */

public interface SubscriptionState extends State {

  public static final int STATUS_WTS_SUBSCRIPTION_ERROR = -5;
  public static final int STATUS_WTS_UNSUBSCRIPTION_ERROR = -4;
  public static final int STATUS_UNEXPECTED_MESSAGE = -3;
  public static final int STATUS_SMSC_NOT_FOUND = -2;
  public static final int STATUS_SYSTEM_ERROR = -1;

  public static final int STATUS_ABONENT_SUBSCRIBED = 0;
  public static final int STATUS_ABONENT_UNSUBSCRIBED = 1;
  public static final int STATUS_ACCEPTED = 2;


  public int subscribeAbonent(String abonentAddress, boolean isAbonentExternal);

  public int unsubscribeAbonent(String abonentAddress, boolean isAbonentExternal);

  public int handleWTSResponse(String abonentAddress, long code, int status);

  public int handleWTSSendError(String abonentAddress);
}
