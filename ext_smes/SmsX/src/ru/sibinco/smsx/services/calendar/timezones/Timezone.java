package ru.sibinco.smsx.services.calendar.timezones;

import ru.aurorisoft.smpp.Address;
import ru.aurorisoft.smpp.SMPPAddressException;

import java.util.*;

/**
 * User: artem
 * Date: 01.08.2006
 */

public final class Timezone {

  private final String name;
  private Set routes = new TreeSet();
  private final Set masks = new HashSet();

  public Timezone(final String name) {
    this.name = name;
  }

  public String getName() {
    return name;
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
    String result = phoneMask.replace('.', '%');
    result = result.replaceAll("%", "\\\\.");
    result = result.replace('?', '.');

    if (result.charAt(0)=='+')
      result = "\\+" + result.substring(1);
    return result;
  }

  public void addMasks(Collection masks) {
    this.masks.addAll(masks);
  }

  public boolean allowNumber(String number) {
    try {
      final Address addr = new Address(number);
      number = "." + addr.getTon() + "." + addr.getNpi() + "." + addr.getAddress();
    } catch (SMPPAddressException e) {
      e.printStackTrace();
      return false;
    }

    for (Iterator iterator = masks.iterator(); iterator.hasNext();) {
      String mask = (String) iterator.next();
      if (number.matches(mask))
        return true;
    }
    return false;
  }
}
