package ru.sibinco.lib;


/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 25.02.2004
 * Time: 19:38:52
 */
public class SibincoException extends Exception
{
  public SibincoException(String message)
  {
    super(message);
  }

  public SibincoException(Throwable cause)
  {
    super(cause);
  }

  public SibincoException(String message, Throwable cause)
  {
    super(message, cause);
  }
}
