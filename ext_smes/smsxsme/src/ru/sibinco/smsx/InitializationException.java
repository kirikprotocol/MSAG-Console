package ru.sibinco.smsx;

/**
 * User: artem
 * Date: 28.06.2007
 */

public class InitializationException extends RuntimeException{
  public InitializationException() {
  }

  public InitializationException(String message) {
    super(message);
  }

  public InitializationException(Throwable cause) {
    super(cause);
  }

  public InitializationException(String message, Throwable cause) {
    super(message, cause);
  }
}
