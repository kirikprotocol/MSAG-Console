package ru.sibinco.smsx;

/**
 * User: artem
 * Date: Sep 4, 2007
 */

public class SmeException extends Exception {
  SmeException() {
  }

  SmeException(String message) {
    super(message);
  }

  SmeException(Throwable cause) {
    super(cause);
  }

  SmeException(String message, Throwable cause) {
    super(message, cause);
  }
}
