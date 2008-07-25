package ru.sibinco.smsx.engine.soaphandler;

import ru.sibinco.smsx.InitializationException;

/**
 * User: artem
 * Date: 10.07.2007
 */

public class SOAPHandlerInitializationException extends Exception {

  public SOAPHandlerInitializationException() {
  }

  public SOAPHandlerInitializationException(String message) {
    super(message);
  }

  public SOAPHandlerInitializationException(Throwable cause) {
    super(cause);
  }

  public SOAPHandlerInitializationException(String message, Throwable cause) {
    super(message, cause);
  }
}
