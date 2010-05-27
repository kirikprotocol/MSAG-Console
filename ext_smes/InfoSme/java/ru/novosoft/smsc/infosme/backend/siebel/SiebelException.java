package ru.novosoft.smsc.infosme.backend.siebel;

/**
 * author: alkhal
 */
public class SiebelException extends Exception {
  public SiebelException() {
    super();
  }

  public SiebelException(String message) {
    super(message);
  }

  public SiebelException(String message, Throwable cause) {
    super(message, cause);
  }

  public SiebelException(Throwable cause) {
    super(cause);
  }
}
