package ru.sibinco.smpp.cmb;

/**
 * Created by IntelliJ IDEA.
 * User: kpv
 * Date: Dec 21, 2006
 * Time: 6:52:09 PM
 */
public class UsageLimitReachedException extends Exception {
  private int limit;
  private String abonent;
  private String date;

  public UsageLimitReachedException(String abonent, int limit, String date) {
    super();
    this.limit = limit;
    this.abonent = abonent;
    this.date = date;
  }

  public String getMessage() {
    StringBuffer sb = new StringBuffer();
    sb.append("Abonent ").
        append(abonent).
        append(" attempts limit (").
        append(limit).
        append(") reached. Date=").
        append(date);
    return sb.toString();
  }

  public int getLimit() {
    return limit;
  }
}
