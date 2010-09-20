package mobi.eyeline.informer.util;

import java.util.*;


/**
 * Created by artem
 */
public class Functions {


  

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
   * Returns string contains all emenetns of array with specified delimeter
   *
   * @param array     array
   * @param delimeter delimeter
   * @return string
   */
  public static String arrayToString(final Object[] array, final String delimeter) {
    if (array.length == 0) {
      return "";
    }
    StringBuilder result = new StringBuilder(10 * array.length);
    for (int i = 0; i < array.length - 1; i++) {
      result.append(array[i].toString()).append(delimeter);
    }
    result.append(array[array.length - 1].toString());

    return result.toString();
  }

  /**
   * Parses string to values by StringTokenizer and adds values to collection in order returned by StringTokenizer.
   *
   * @param checkedTasksSet values will be added to this collection
   * @param values          string to tokenize
   * @param delimeter       string values delimiter
   * @param trimValues      if true, each value will be trimmed before add to collection
   */
  public static void addValuesToCollection(Collection checkedTasksSet, final String values, final String delimeter, boolean trimValues) {
    for (StringTokenizer i = new StringTokenizer(values, delimeter, false); i.hasMoreTokens();) {
      checkedTasksSet.add(trimValues ? i.nextToken().trim() : i.nextToken());
    }
  }

  private static long filenameCounter = 0;

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
   * Compares 2 strings
   *
   * @param s1 first string
   * @param s2 second string
   * @return true if first string is equals to second string
   * @deprecated bad function name
   */
  public static boolean compareStrs(String s1, String s2) {
    return (s1 == null && s2 == null) || (s1 != null && s2 != null && s1.equals(s2));
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
