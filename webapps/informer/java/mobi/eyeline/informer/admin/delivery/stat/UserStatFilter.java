package mobi.eyeline.informer.admin.delivery.stat;

import java.util.Date;

/**
 * @author Artem Snopkov
 */
public class UserStatFilter {

  private Date fromDate;
  private Date tillDate;
  private String user;

  public Date getFromDate() {
    return fromDate;
  }

  public void setFromDate(Date fromDate) {
    this.fromDate = fromDate;
  }

  public Date getTillDate() {
    return tillDate;
  }

  public void setTillDate(Date tillDate) {
    this.tillDate = tillDate;
  }

  public String getUser() {
    return user;
  }

  public void setUser(String user) {
    this.user = user;
  }
}
