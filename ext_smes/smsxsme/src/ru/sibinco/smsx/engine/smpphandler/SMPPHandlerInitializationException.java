package ru.sibinco.smsx.engine.smpphandler;

import ru.sibinco.smsx.InitializationException;

/**
 * User: artem
 * Date: 02.07.2007
 */

public class SMPPHandlerInitializationException extends InitializationException {
  public SMPPHandlerInitializationException() {
  }

  public SMPPHandlerInitializationException(String message) {
    super(message);
  }

  public SMPPHandlerInitializationException(Throwable cause) {
    super(cause);
  }

  public SMPPHandlerInitializationException(String message, Throwable cause) {
    super(message, cause);
  }
}
