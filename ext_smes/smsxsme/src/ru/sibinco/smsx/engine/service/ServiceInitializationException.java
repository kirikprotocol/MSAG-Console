package ru.sibinco.smsx.engine.service;

import ru.sibinco.smsx.InitializationException;

/**
 * User: artem
 * Date: Jul 1, 2007
 */

public class ServiceInitializationException extends InitializationException {
  public ServiceInitializationException() {
  }

  public ServiceInitializationException(String message) {
    super(message);
  }

  public ServiceInitializationException(Throwable cause) {
    super(cause);
  }

  public ServiceInitializationException(String message, Throwable cause) {
    super(message, cause);
  }
}
