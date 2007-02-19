package ru.sibinco.otasme.utils;

import ru.sibinco.otasme.InitializationException;

import java.io.IOException;
import java.util.Properties;

/**
 * User: artem
 * Date: Jul 26, 2006
 */

public final class Utils {

  public static long loadLong(final Properties config, final String propertyName) throws InitializationException {

    final String propertyStr = config.getProperty(propertyName);
    if (propertyStr == null)
      throw new InitializationException(propertyName + " property not found ");

    long property;
    try {
      property = Long.parseLong(propertyStr);
    } catch (NumberFormatException e) {
      throw new InitializationException("uncorrect" + propertyName + "property");
    }

    return property;
  }

  public static int loadInt(final Properties config, final String propertyName) throws InitializationException {
    final String propertyStr = config.getProperty(propertyName);
    if (propertyStr == null)
      throw new InitializationException(propertyName + " property not found ");

    int property;
    try {
      property = Integer.parseInt(propertyStr);
    } catch (NumberFormatException e) {
      throw new InitializationException("uncorrect" + propertyName + "property");
    }

    return property;
  }

  public static String loadString(final Properties config, final String propertyName) throws InitializationException {
    final String propertyStr = config.getProperty(propertyName);
    if (propertyStr == null)
      throw new InitializationException(propertyName + " property not found ");
    return propertyStr;
  }

  public static boolean loadBoolean(final Properties config, final String propertyName) throws InitializationException {

    final String propertyStr = config.getProperty(propertyName);
    if (propertyStr == null)
      throw new InitializationException(propertyName + " property not found ");

    boolean property;
    try {
      property = Boolean.valueOf(propertyStr).booleanValue();
    } catch (NumberFormatException e) {
      throw new InitializationException("uncorrect" + propertyName + "property");
    }

    return property;
  }

  public static Properties loadConfig(final String source) throws InitializationException {
    final Properties config = new Properties();
    try {
      config.load(ClassLoader.getSystemResourceAsStream(source));
    } catch (IOException e) {
      throw new InitializationException("Can't load config: " + e);
    }
    return config;
  }
}
