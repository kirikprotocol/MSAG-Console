package mobi.eyeline.informer.util.config;

import mobi.eyeline.informer.util.Functions;

import java.text.SimpleDateFormat;
import java.util.Collection;
import java.util.Date;
import java.util.List;
import java.util.Properties;


/**
 * @author Aleksandr Khalitov
 *         <p/>
 *         Parameter of XmlConfig
 */

public class XmlConfigParam implements Cloneable {

  public static enum Type {
    STRING, INT, BOOL
  }

  private String name;
  private String value;
  private final Properties attributes = new Properties();
  private Type type = Type.STRING;

  /**
   * Creates new instance of XmlConfigParam with specified name
   *
   * @param name param name
   */
  public XmlConfigParam(String name) {
    this.name = name;
  }

  /**
   * Creates new instance of XmlConfigParam with specified name and value
   *
   * @param name  param name
   * @param value param value
   * @param type  param type
   */
  public XmlConfigParam(String name, String value, Type type) {
    this.name = name;
    this.value = value;
    this.type = type;
  }

  /**
   * Returns parameter type
   *
   * @return parameter type
   */
  public Type getType() {
    return type;
  }

  /**
   * Returns parameter name
   *
   * @return parameter name
   */
  public String getName() {
    return name;
  }

  /**
   * Sets parameter name
   *
   * @param name new parameter name
   */
  public void setName(String name) {
    this.name = name;
  }

  String getValue() {
    return value;
  }


  // GETTERS ===================================================================


  /**
   * Returns parameter value as String
   *
   * @return strign value
   */
  public String getString() {
    return value;
  }

  /**
   * Returns parameter value as int
   *
   * @return int value
   * @throws XmlConfigException if param value not int
   */
  public int getInt() throws XmlConfigException {
    return ConfigTools.getInt(name, value);
  }

  /**
   * Returns parameter value as long
   *
   * @return long value
   * @throws XmlConfigException if param value not long
   */
  public long getLong() throws XmlConfigException {
    return ConfigTools.getLong(name, value);
  }

  /**
   * Returns parameter value as date with specified date format
   *
   * @param dateFormat date format
   * @return date value
   * @throws XmlConfigException if can't parse param value
   */
  public Date getDate(String dateFormat) throws XmlConfigException {
    return ConfigTools.getDate(name, value, dateFormat);
  }

  /**
   * Returns parameter value as boolean
   *
   * @return boolean value
   * @throws XmlConfigException if param value does not match boolean
   */
  public boolean getBool() throws XmlConfigException {
    return ConfigTools.getBool(name, value);
  }

  /**
   * Returns parameter value as String list
   *
   * @param delimiter delimiter
   * @return string list value
   * @throws XmlConfigException error reading config
   */
  public String[] getStringArray(String delimiter) throws XmlConfigException {
    return ConfigTools.getStringArray(name, value, delimiter);
  }

  /**
   * Returns parameter value as String list
   *
   * @param delimiter delimiter
   * @return string list value
   * @throws XmlConfigException error reading config
   */
  public List<String> getStringList(String delimiter) throws XmlConfigException {
    return ConfigTools.getStringList(name, value, delimiter);
  }

  /**
   * Returns parameter value as long list
   *
   * @param delimiter delimiter
   * @return long array
   * @throws XmlConfigException if param value does not match long[]
   */
  public long[] getLongArray(String delimiter) throws XmlConfigException {
    return ConfigTools.getLongArray(name, value, delimiter);
  }

  /**
   * Returns parameter value as int list
   *
   * @param delimiter delimiter
   * @return int list
   * @throws XmlConfigException if param value does not match int[]
   */
  public int[] getIntArray(String delimiter) throws XmlConfigException {
    return ConfigTools.getIntArray(name, value, delimiter);
  }

  public String getAttribute(String attrName) {
    return attributes.getProperty(attrName);
  }


  // SETTERS ===================================================================


  /**
   * Set string value to parameter
   *
   * @param string new value
   */
  public void setString(final String string) {
    value = string;
    type = Type.STRING;
  }

  /**
   * Set int value to parameter
   *
   * @param value new value
   */
  public void setInt(int value) {
    this.value = String.valueOf(value);
    type = Type.INT;
  }

  /**
   * Set long value to parameter
   *
   * @param value new value
   */
  public void setLong(long value) {
    this.value = String.valueOf(value);
    type = Type.INT;
  }

  /**
   * Set int value to parameter
   *
   * @param date       new value
   * @param dateFormat date format
   */
  public void setDate(Date date, String dateFormat) {
    if (dateFormat == null)
      throw new IllegalArgumentException("Date format is empty");

    if (date != null) {
      final SimpleDateFormat df = new SimpleDateFormat(dateFormat);
      value = df.format(date);
    } else
      value = null;
    type = Type.STRING;
  }

  /**
   * Set bool value to parameter
   *
   * @param value new value
   */
  public void setBool(boolean value) {
    this.value = String.valueOf(value);
    type = Type.BOOL;
  }

  /**
   * Set string list value to parameter
   *
   * @param stringList new value
   * @param delimiter  values delimiter
   * @throws XmlConfigException error writting config
   */
  public void setStringList(Collection stringList, String delimiter) throws XmlConfigException {
    value = Functions.collectionToString(stringList, delimiter);
    type = Type.STRING;
  }

  /**
   * Set string list value to parameter
   *
   * @param stringList new value
   * @param delimiter  values delimiter
   */
  public void setStringArray(String[] stringList, String delimiter) {
    value = arrayToString(stringList, delimiter);
    type = Type.STRING;
  }

  /**
   * Set ilong list value to parameter
   *
   * @param values    new values
   * @param delimiter delimiter
   */
  public void setLongArray(long[] values, String delimiter) {
    Object[] os = new Object[values.length];
    for (int i = 0; i < values.length; i++) {
      os[i] = values[i];
    }
    this.value = arrayToString(os, delimiter);
    type = Type.STRING;
  }

  /**
   * Set int list value to parameter
   *
   * @param values    new value
   * @param delimiter values delimiter
   */
  public void setIntArray(int[] values, String delimiter) {
    Object[] os = new Object[values.length];
    for (int i = 0; i < values.length; i++) {
      os[i] = values[i];
    }
    this.value = arrayToString(os, delimiter);
    type = Type.STRING;
  }

  private static String arrayToString(final Object[] array, final String delimeter) {
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
   * Set int list value to parameter
   *
   * @param values    new value
   * @param delimiter values delimiter
   */
  public void setIntList(Collection<Integer> values, String delimiter) {
    this.value = Functions.collectionToString(values, delimiter);
    type = Type.STRING;
  }

  public void setAttribute(String attrName, String value) {
    this.attributes.setProperty(attrName, value);
  }

  public boolean equals(Object o) {
    if (o instanceof XmlConfigParam) {
      XmlConfigParam p = (XmlConfigParam) o;
      return p.type == type && p.name.equals(name) && p.value.equals(value);
    }
    return false;
  }

  public Object clone() throws CloneNotSupportedException {
    XmlConfigParam p = (XmlConfigParam) super.clone();
    p.name = name;
    p.value = value;
    p.type = type;
    p.attributes.putAll(attributes);
    return p;
  }

}
