package ru.sibinco.otasme.engine.service;

import com.eyeline.sme.utils.statemachine.Event;

/**
 * User: artem
 * Date: Oct 5, 2007
 */

class StateEvent implements Event {
  public static final byte TYPE_SET_WTS_SUBSCRIPTION_STATE = 0;
  public static final byte TYPE_SET_WTS_UNSUBSCRIPTION_STATE = 1;
  public static final byte TYPE_REMOVE_STATE = 2;

  private final byte type;
  private final String abonentAddress;

  public StateEvent(byte type, String abonentAddress) {
    this.type = type;
    this.abonentAddress = abonentAddress;
  }

  public byte getType() {
    return type;
  }

  public String getAbonentAddress() {
    return abonentAddress;
  }
}
