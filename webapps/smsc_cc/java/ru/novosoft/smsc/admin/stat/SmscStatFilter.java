package ru.novosoft.smsc.admin.stat;

import java.util.Date;

/**
 * author: Aleksandr Khalitov
 */
public class SmscStatFilter {

  private Date from;

  private Date till;


  public Date getFrom() {
    return from;
  }

  public void setFrom(Date from) {
    this.from = from;
  }

  public Date getTill() {
    return till;
  }

  public void setTill(Date till) {
    this.till = till;
  }
}
