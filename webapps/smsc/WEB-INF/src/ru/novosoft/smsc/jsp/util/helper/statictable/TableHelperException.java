package ru.novosoft.smsc.jsp.util.helper.statictable;

/**
 * User: artem
 * Date: 15.01.2007
 */
public class TableHelperException extends Exception{

  public TableHelperException() {
  }

  public TableHelperException(String s) {
    super(s);
  }

  public TableHelperException(Throwable throwable) {
    super(throwable);
  }

  public TableHelperException(String s, Throwable throwable) {
    super(s, throwable);
  }
}
