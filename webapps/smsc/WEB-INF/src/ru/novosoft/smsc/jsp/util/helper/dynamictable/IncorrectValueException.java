package ru.novosoft.smsc.jsp.util.helper.dynamictable;

/**
 * User: artem
 * Date: 18.01.2007
 */

public class IncorrectValueException extends Exception{
  public IncorrectValueException() {
  }

  public IncorrectValueException(String s) {
    super(s);
  }

  public IncorrectValueException(Throwable throwable) {
    super(throwable);
  }

  public IncorrectValueException(String s, Throwable throwable) {
    super(s, throwable);
  }
}
