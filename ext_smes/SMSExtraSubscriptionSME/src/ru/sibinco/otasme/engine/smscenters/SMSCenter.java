package ru.sibinco.otasme.engine.smscenters;

import org.apache.log4j.Category;

import java.util.*;

/**
 * User: artem
 * Date: 01.08.2006
 */

public final class SMSCenter {

  private static final Category log = Category.getInstance(SMSCenter.class);

  private final String number;
  private Set routes = new TreeSet();
  private final Set masks = new HashSet();

  public SMSCenter(final String number) {
    this.number = number;
  }

  public String getNumber() {
    return number;
  }

  public void addRoute(String route) {
    routes.add(route.replace('^', '.'));
  }

  public boolean hasRoute(String route) {
    return routes.contains(route);
  }

  public Set getMasks() {
    return masks;
  }

  public void addMask(String mask) {
    masks.add(preparePhoneMask(mask.replace('^', '.')));
  }

  private static String preparePhoneMask(final String phoneMask) {
    try {
      String result = null;

      if (phoneMask.startsWith("."))
        result = phoneMask.split("\\.")[3];

      if (phoneMask.startsWith("+"))
        result = phoneMask.substring(1);

      result = result.replace('?', '.');

      return result;
    } catch (Throwable e) {
      log.error("Can't add mask " + phoneMask, e);
      return "";
    }
//    String result = phoneMask.replace('.', '%');
//    result = result.replaceAll("%", "\\\\.");

//
//    if (result.charAt(0)=='+')
//      result = "\\+" + result.substring(1);

  }


  public boolean allowNumber(String number) {
    final String num = preparePhoneMask(number);
    for (Iterator iterator = masks.iterator(); iterator.hasNext();) {
      String mask = (String) iterator.next();
      if (num.matches(mask)) {
        System.out.println("Maches mask: " + mask);
        return true;
      }
    }
    return false;
  }
}
