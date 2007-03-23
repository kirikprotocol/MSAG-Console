package ru.sibinco.smpp.ub_sme.inbalance;

/**
 * Created by IntelliJ IDEA.
 * User: danger
 * Date: 07.12.2006
 * Time: 14:01:30
 * To change this template use File | Settings | File Templates.
 */
public class InBalanceClientException extends Exception {

  public InBalanceClientException(String message, Throwable cause) {
    super(message, cause);
  }

  public InBalanceClientException(Throwable cause) {
    super(cause);
  }

  public InBalanceClientException(String message) {
    super(message);
  }
}
