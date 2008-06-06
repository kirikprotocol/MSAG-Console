package ru.sibinco.smsx;

import java.util.HashMap;

/**
 * User: artem
 * Date: 11.07.2007
 */

public class Statuses {

  private static final HashMap statuses = new HashMap();

  static {
    statuses.put(new Integer(-200), "Destination abonent not registered");
    statuses.put(new Integer(-100), "Wrong send date");
    statuses.put(new Integer(-6), "Destination abonent in black list");
    statuses.put(new Integer(-5), "Not MTS abonent");
    statuses.put(new Integer(-4), "Invalid message id");
    statuses.put(new Integer(-3), "Message not found");
    statuses.put(new Integer(-2), "System error");
    statuses.put(new Integer(-1), "Delivery error");
    statuses.put(new Integer(0), "Accepted");
    statuses.put(new Integer(1), "Delivered");
  }

  public static String getStatus(int code) {
    return (String)statuses.get(new Integer(code));
  }
}
