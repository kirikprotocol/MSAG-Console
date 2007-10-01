package ru.sibinco.smsx.network.smscconsole;

/**
 * User: artem
 * Date: 17.07.2007
 */

public class SmscConsoleClientException extends Exception{
  SmscConsoleClientException() {
  }

  SmscConsoleClientException(String message) {
    super(message);
  }

  SmscConsoleClientException(Throwable cause) {
    super(cause);
  }

  SmscConsoleClientException(String message, Throwable cause) {
    super(message, cause);
  }
}
