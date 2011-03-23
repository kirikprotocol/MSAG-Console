package mobi.eyeline.informer.util;

import java.util.*;


/**
 * Created by artem
 */
public class Functions {

  private Functions() {}

  /**
   * Returns string contains all emenetns of collection with specified delimeter
   *
   * @param collection collection
   * @param delimeter  delimeter
   * @return string
   */
  public static String collectionToString(final Collection collection, final String delimeter) {
    StringBuilder result = new StringBuilder(10 * collection.size());
    for (Iterator i = collection.iterator(); i.hasNext();) {
      result.append(i.next().toString());
      if (i.hasNext())
        result.append(delimeter);
    }
    return result.toString();
  }

  /**
   * Set hour, minute, seconds and milliseconds to 0
   *
   * @param dateTime ooriginal date
   * @return new date
   */
  public static Date truncateTime(Date dateTime) {
    Calendar calendar = new GregorianCalendar();
    calendar.setTime(dateTime);
    calendar.set(Calendar.HOUR_OF_DAY, 0);
    calendar.set(Calendar.MINUTE, 0);
    calendar.set(Calendar.SECOND, 0);
    calendar.set(Calendar.MILLISECOND, 0);
    return calendar.getTime();
  }

  /**
   * Returns file.encoding
   *
   * @return file.encoding
   */
  public static String getLocaleEncoding() {
    return System.getProperty("file.encoding", "WINDOWS-1251");
  }

  /**
   * Converts time from one timezone to another
   *
   * @param time         time
   * @param fromTimezone source timezone
   * @param toTimezone   destinatin timezone
   * @return converted time
   * @throws NullPointerException if any of arguments is null
   */
  public static Date convertTime(final Date time, TimeZone fromTimezone, TimeZone toTimezone) {
    // Find time difference
    final long d1 = fromTimezone.getOffset(time.getTime());
    final long d2 = toTimezone.getOffset(time.getTime());

    Calendar calend = Calendar.getInstance();
    calend.setTimeInMillis(time.getTime() + d2 - d1);
    return calend.getTime();
  }
}
