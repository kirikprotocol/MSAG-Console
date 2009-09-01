package ru.novosoft.smsc.infosme.backend.siebel;

/**
 * author: alkhal
 */
public class SiebelDataProviderException extends Exception{
  public SiebelDataProviderException() {
    super();
  }

  public SiebelDataProviderException(String message) {
    super(message);
  }

  public SiebelDataProviderException(String message, Throwable cause) {
    super(message, cause);
  }

  public SiebelDataProviderException(Throwable cause) {
    super(cause);
  }
}
