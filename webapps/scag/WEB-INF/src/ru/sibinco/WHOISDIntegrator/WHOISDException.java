package ru.sibinco.WHOISDIntegrator;


/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 13.03.2006
 * Time: 11:22:07
 * To change this template use File | Settings | File Templates.
 */
public class WHOISDException extends Exception{
  int lineNumber;
  public WHOISDException(String message) {
    super(message);
  }
  public WHOISDException(String message, int lineNumber) {
    super(message);
    this.lineNumber = lineNumber;
  }
  public int getLineNumber() {
    return lineNumber;
  }
}
