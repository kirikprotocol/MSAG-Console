package ru.sibinco.smpp.ub_sme.inman;

/**
 * Created by IntelliJ IDEA.
 * User: danger
 * Date: 07.12.2006
 * Time: 14:01:30
 * To change this template use File | Settings | File Templates.
 */
public class InManClientException extends Exception {

  public InManClientException(String message, Throwable cause) {
    super(message, cause);
  }

  public InManClientException(Throwable cause) {
    super(cause);
  }

  public InManClientException(String message) {
    super(message);
  }
}
