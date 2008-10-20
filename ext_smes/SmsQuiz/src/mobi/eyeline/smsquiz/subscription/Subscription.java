package mobi.eyeline.smsquiz.subscription;

import java.util.Date;


public class Subscription {
  private String address;
  private Date startDate;
  private Date endDate;

  public Subscription() {
    startDate = new Date();
  }

  public String getAddress() {
    return address;
  }

  public void setAddress(String address) {
    this.address = address;
  }

  public Date getStartDate() {
    return startDate;
  }

  public void setStartDate(Date startDate) {
    this.startDate = startDate;
  }

  public Date getEndDate() {
    return endDate;
  }

  public void setEndDate(Date endDate) {
    this.endDate = endDate;
  }

  public String toString() {
    String res = "";
    if (address != null)
      res += address;
    if (startDate != null)
      res += " " + startDate;
    if (endDate != null)
      res += " " + endDate;
    return res;
  }
}
