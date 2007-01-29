package ru.sibinco.smsx.services;

/**
 * User: artem
 * Date: 25.01.2007
 */
public final class ProcessException extends Exception{
  public ProcessException() {
  }

  public ProcessException(String s) {
    super(s);
  }

  public ProcessException(Throwable throwable) {
    super(throwable);
  }

  public ProcessException(String s, Throwable throwable) {
    super(s, throwable);
  }
}
