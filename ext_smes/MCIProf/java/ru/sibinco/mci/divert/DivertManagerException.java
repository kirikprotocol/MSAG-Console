package ru.sibinco.mci.divert;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 25.10.2004
 * Time: 16:12:07
 * To change this template use File | Settings | File Templates.
 */
public class DivertManagerException extends Exception
{
  public final static int UNKNOWN       = -1;
  public final static int CONNECT       = 1;
  public final static int COMMUNICATION = 2;
  public final static int NOT_ACCEPTED  = 3;

  protected int code = UNKNOWN;

  public DivertManagerException() {
  }
  public DivertManagerException(int cause) {
    this.code = cause;
  }
  public DivertManagerException(String message, int code) {
    super(message); this.code = code;
  }
  public DivertManagerException(String message, Throwable cause, int code) {
    super(message, cause); this.code = code;
  }
  public DivertManagerException(Throwable cause, int code) {
    super(cause);  this.code = code;
  }
}
