package ru.novosoft.smsc.infosme.backend.siebel;

/**
 * author: alkhal
 */
public class IntegrationDataSourceException extends Exception{
  public IntegrationDataSourceException() {
    super();
  }

  public IntegrationDataSourceException(String message) {
    super(message);
  }

  public IntegrationDataSourceException(String message, Throwable cause) {
    super(message, cause);
  }

  public IntegrationDataSourceException(Throwable cause) {
    super(cause);
  }
}
