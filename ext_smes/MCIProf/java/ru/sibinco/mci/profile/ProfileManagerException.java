package ru.sibinco.mci.profile;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 26.10.2004
 * Time: 17:02:02
 * To change this template use File | Settings | File Templates.
 */
public class ProfileManagerException extends Exception
{
  public final static int UNKNOWN  = -1;
  public final static int DB_ERROR = 1;
  
  protected int code = UNKNOWN;

  public ProfileManagerException() {
  }
  public ProfileManagerException(int code) {
    this.code = code;
  }
  public ProfileManagerException(String message, int code) {
    super(message); this.code = code;
  }
  public ProfileManagerException(String message, Throwable cause, int code) {
    super(message, cause);  this.code = code;
  }
  public ProfileManagerException(Throwable cause, int code) {
    super(cause); this.code = code;
  }
}
