package ru.novosoft.smsc.web;

/**
 * author: alkhal
 */
public class InitException extends Exception {

  public InitException() {
  }

  public InitException(String message) {
    super(message);
  }

  public InitException(String message, Throwable cause) {
    super(message, cause);
  }

  public InitException(Throwable cause) {
    super(cause);
  }
}
