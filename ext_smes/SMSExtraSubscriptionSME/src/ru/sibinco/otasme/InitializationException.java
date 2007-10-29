package ru.sibinco.otasme;

public final class InitializationException extends RuntimeException {
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
