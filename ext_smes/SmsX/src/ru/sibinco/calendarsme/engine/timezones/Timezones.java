package ru.sibinco.calendarsme.engine.timezones;

import java.util.SortedMap;
import java.util.Iterator;

/**
 * User: artem
 * Date: Aug 2, 2006
 */

public class Timezones {
  private final SortedMap abonentsTimezones;
  private final String defaultTimezone;

  public Timezones(final SortedMap abonentsTimezones, final String defaultTimezone) {
    this.abonentsTimezones = abonentsTimezones;
    this.defaultTimezone = defaultTimezone;
  }

  public String getTimezoneByAbonent(final String abonentAddr) throws TimezoneNotFoundException {
    for (Iterator iter = abonentsTimezones.keySet().iterator(); iter.hasNext();) {
      final String phoneMask = (String)iter.next();
      if (abonentAddr.matches(phoneMask))
        return (String)abonentsTimezones.get(phoneMask);
    }
    if (defaultTimezone != null)
      return defaultTimezone;

    throw new TimezoneNotFoundException();
  }

  public class TimezoneNotFoundException extends Exception {
  }
}
