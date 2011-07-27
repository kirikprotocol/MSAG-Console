package mobi.eyeline.informer.util.config;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.LinkedList;
import java.util.List;
import java.util.StringTokenizer;

/**
 * @author Aleksandr Khalitov
 */
class ConfigTools {

  static long getLong(String key, String propertyStr) throws XmlConfigException {
    if (propertyStr == null)
      throw new XmlConfigException("Value of " + key + " is empty");

    try {
      return Long.parseLong(propertyStr);
    } catch (NumberFormatException e) {
      throw new XmlConfigException("Incorrect " + key + " type. It must be long.");
    }
  }

  static int getInt(String key, String propertyStr) throws XmlConfigException {
    if (propertyStr == null)
      throw new XmlConfigException("Value of " + key + " is empty");

    try {
      return Integer.parseInt(propertyStr);
    } catch (NumberFormatException e) {
      throw new XmlConfigException("Incorrect " + key + " type. It must be int.");
    }
  }

  static boolean getBool(String key, String propertyStr) throws XmlConfigException {
    if (propertyStr == null)
      throw new XmlConfigException("Value of " + key + " is empty");

    try {
      return Boolean.valueOf(propertyStr);
    } catch (NumberFormatException e) {
      throw new XmlConfigException("Incorrect " + key + " type. It must be bool (true/false).");
    }
  }

  static Date getDate(String key, String propertyStr, String dateFormat) throws XmlConfigException {
    if (propertyStr == null)
      throw new XmlConfigException("Value of " + key + " is empty");

    if (dateFormat == null)
      throw new IllegalArgumentException("Date format is empty");

    final SimpleDateFormat df = new SimpleDateFormat(dateFormat);
    try {
      return df.parse(propertyStr);
    } catch (ParseException ex) {
      throw new XmlConfigException("Incorrect " + key + " type. It must be date with format " + dateFormat);
    }
  }

  static int[] getIntArray(String key, String propertyStr, String delimiter) throws XmlConfigException {
    if (propertyStr == null)
      throw new XmlConfigException("Value of " + key + " is empty");

    if (delimiter == null)
      throw new IllegalArgumentException("Delimiter is empty");

    final StringTokenizer st = new StringTokenizer(propertyStr, delimiter);
    final int[] result = new int[st.countTokens()];

    try {
      for (int i = 0; i < result.length && st.hasMoreTokens(); i++) {
        result[i] = Integer.parseInt(st.nextToken().trim());
      }
    } catch (NumberFormatException e) {
      throw new XmlConfigException("Invalid " + key + " list type. It must be int[] with delimiter " + delimiter);
    }

    return result;
  }

  static long[] getLongArray(String key, String propertyStr, String delimiter) throws XmlConfigException {
    if (propertyStr == null)
      throw new XmlConfigException("Value of " + key + " is empty");

    if (delimiter == null)
      throw new IllegalArgumentException("Delimiter is empty");

    final StringTokenizer st = new StringTokenizer(propertyStr, delimiter);
    final long[] result = new long[st.countTokens()];

    try {
      for (int i = 0; i < result.length && st.hasMoreTokens(); i++)
        result[i] = Long.parseLong(st.nextToken().trim());
    } catch (NumberFormatException e) {
      throw new XmlConfigException("Invalid " + key + " list type. It must be long[] with delimiter " + delimiter);
    }

    return result;
  }

  static String[] getStringArray(String key, String propertyStr, String delimiter) throws XmlConfigException {
    if (propertyStr == null)
      throw new XmlConfigException("Value of " + key + " is empty");

    if (delimiter == null)
      throw new IllegalArgumentException("Delimiter is empty");

    final StringTokenizer st = new StringTokenizer(propertyStr, delimiter);
    final String[] result = new String[st.countTokens()];

    for (int i = 0; i < result.length && st.hasMoreTokens(); i++) {
      result[i] = st.nextToken();
    }
    return result;
  }

  static List<String> getStringList(String key, String propertyStr, String delimiter) throws XmlConfigException {
    if (propertyStr == null)
      throw new XmlConfigException("Value of " + key + " is empty");

    if (delimiter == null)
      throw new IllegalArgumentException("Delimiter is empty");

    final List<String> result = new LinkedList<String>();

    final StringTokenizer st = new StringTokenizer(propertyStr, delimiter);

    while (st.hasMoreTokens())
      result.add(st.nextToken());

    return result;
  }

}
