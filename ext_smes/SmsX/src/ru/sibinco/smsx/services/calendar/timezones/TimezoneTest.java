package ru.sibinco.smsx.services.calendar.timezones;

import java.util.Calendar;
import java.util.Date;
import java.util.TimeZone;

/**
 * User: artem
 * Date: Aug 2, 2006
 */

public class TimezoneTest {
  public static void main(String[] args) {

    final long d1 = TimeZone.getTimeZone("Europe/Moscow").getOffset(new Date().getTime());
    final long d2 = TimeZone.getDefault().getOffset(new Date().getTime());
    System.out.println(d1 - d2);
    Calendar calend = Calendar.getInstance();
    calend.setTimeInMillis(new Date().getTime() +  d2 - d1);
    System.out.println(calend.getTime());
  }
}
