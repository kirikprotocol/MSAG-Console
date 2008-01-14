package ru.sibinco.smpp.ub_sme;

public class InitializationException extends RuntimeException {
  public InitializationException(String message) {
    super(message);
  }

  public InitializationException(String message, Throwable cause) {
    super(message, cause);
  }

  public InitializationException(Throwable cause) {
    super(cause);
  }
}
