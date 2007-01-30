package ru.sibinco.smpp.cmb;

/**
 * Created by IntelliJ IDEA.
 * User: kpv
 * Date: Dec 21, 2006
 * Time: 6:53:24 PM
 */
public class BalanceLimitException extends Exception {
  private String abonent;

  public BalanceLimitException(String abonent) {
    super();
    this.abonent = abonent;
  }

  public String getMessage() {
    return "Abonent "+abonent+" balance is positive.";
  }
}
