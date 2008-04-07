package com.eyeline.sponsored.utils;

import java.util.Calendar;
import java.util.Date;
import java.util.TimeZone;

/**
 * User: artem
 * Date: 30.01.2008
 */

public class CalendarUtils {
  
  public static Date convertFromTZToLocal(Date date, TimeZone tz) {
    final long d1 = tz.getOffset(date.getTime());
    final long d2 = TimeZone.getDefault().getOffset(date.getTime());
    return new Date(date.getTime() + d2 - d1);    
  }

  public static Date convertFromLocalToTZ(Date date, TimeZone tz) {
    final long d1 = tz.getOffset(date.getTime());
    final long d2 = TimeZone.getDefault().getOffset(date.getTime());
    return new Date(date.getTime() + d1 - d2);    
  }

  public static Date getDayStart(Date date) {
    final Calendar c = Calendar.getInstance();
    c.setTime(date);
    c.set(Calendar.HOUR_OF_DAY, 0);
    c.set(Calendar.MINUTE, 0);
    c.set(Calendar.SECOND, 0);
    c.set(Calendar.MILLISECOND, 0);
    return c.getTime();
  }

  public static long getDayStartInMillis(Date date) {
    final Calendar c = Calendar.getInstance();
    c.setTime(date);
    c.set(Calendar.HOUR_OF_DAY, 0);
    c.set(Calendar.MINUTE, 0);
    c.set(Calendar.SECOND, 0);
    c.set(Calendar.MILLISECOND, 0);
    return c.getTimeInMillis();
  }

  public static Date getPrevDayStart(Date date) {
    final Calendar c = Calendar.getInstance();
    c.setTime(date);
    c.set(Calendar.HOUR_OF_DAY, 0);
    c.set(Calendar.MINUTE, 0);
    c.set(Calendar.SECOND, 0);
    c.set(Calendar.MILLISECOND, 0);
    c.set(Calendar.DAY_OF_YEAR, c.get(Calendar.DAY_OF_YEAR) - 1);
    return c.getTime();
  }

  public static Date getNextDayStart(Date date) {
    final Calendar c = Calendar.getInstance();
    c.setTime(date);
    c.set(Calendar.HOUR_OF_DAY, 0);
    c.set(Calendar.MINUTE, 0);
    c.set(Calendar.SECOND, 0);
    c.set(Calendar.MILLISECOND, 0);
    c.set(Calendar.DAY_OF_YEAR, c.get(Calendar.DAY_OF_YEAR) + 1);
    return c.getTime();
  }

  public static Date getNextHourStart(Date date) {
    final Calendar c = Calendar.getInstance();
    c.setTime(date);
    c.set(Calendar.HOUR, c.get(Calendar.HOUR) + 1);
    c.set(Calendar.MINUTE, 0);
    c.set(Calendar.SECOND, 0);
    c.set(Calendar.MILLISECOND, 0);
    return c.getTime();
  }

  public static void main(String[] args) {
//    System.out.println(convertFromTZToLocal(new Date(), TimeZone.getTimeZone("Europe/Moscow")));
//    System.out.println();
//    System.out.println(getDayStart(new Date()));

    Calendar c = Calendar.getInstance(TimeZone.getTimeZone("Asia/Vladivostok"));
    c.setTime(convertFromLocalToTZ(new Date(), TimeZone.getTimeZone("Asia/Vladivostok")));

    System.out.println(c.get(Calendar.DATE));

    c.set(Calendar.HOUR_OF_DAY, 9);
    c.set(Calendar.MINUTE, 0);
    c.set(Calendar.SECOND, 0);

    System.out.println(c.getTime());
  }
}
