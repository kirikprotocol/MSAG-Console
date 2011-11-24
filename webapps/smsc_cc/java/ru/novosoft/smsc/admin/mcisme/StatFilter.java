package ru.novosoft.smsc.admin.mcisme;

import java.util.Date;

/**
 * author: Aleksandr Khalitov
 */
public class StatFilter {

  private Date fromDate;

  private Date tillDate;

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
}
