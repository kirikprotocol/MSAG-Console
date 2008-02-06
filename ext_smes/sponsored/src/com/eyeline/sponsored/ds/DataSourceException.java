package com.eyeline.sponsored.ds;

/**
 *
 * @author artem
 */
public class DataSourceException extends Exception {

  public DataSourceException(Throwable cause) {
    super(cause);
  }

  public DataSourceException(String message, Throwable cause) {
    super(message, cause);
  }

  public DataSourceException(String message) {
    super(message);
  }

  public DataSourceException() {
  }
}
