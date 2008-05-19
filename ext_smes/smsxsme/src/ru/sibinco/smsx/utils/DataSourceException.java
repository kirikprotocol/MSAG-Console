package ru.sibinco.smsx.utils;

/**
 * User: artem
 * Date: 14.05.2008
 */

public class DataSourceException extends Exception {
  public DataSourceException() {
  }

  public DataSourceException(String message) {
    super(message);
  }

  public DataSourceException(String message, Throwable cause) {
    super(message, cause);
  }

  public DataSourceException(Throwable cause) {
    super(cause);
  }
}
