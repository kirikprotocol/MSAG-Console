package ru.novosoft.smsc.admin.console.commands.infosme;

import java.util.Date;
import java.util.Calendar;
import java.util.Set;

/**
 * author: alkhal
 * Date: 27.10.2008
 */
public class Distribution {
  private String file;
  private Date dateBegin;
  private Date dateEnd;
  private Calendar timeBegin;
  private Calendar timeEnd;
  private String address;
  private Boolean txmode;
  private Set days;

  public String getFile() {
    return file;
  }

  public void setFile(String file) {
    this.file = file;
  }

  public Date getDateBegin() {
    return dateBegin;
  }

  public void setDateBegin(Date dateBegin) {
    this.dateBegin = dateBegin;
  }

  public Date getDateEnd() {
    return dateEnd;
  }

  public void setDateEnd(Date dateEnd) {
    this.dateEnd = dateEnd;
  }

  public Calendar getTimeBegin() {
    return timeBegin;
  }

  public void setTimeBegin(Calendar timeBegin) {
    this.timeBegin = timeBegin;
  }

  public Calendar getTimeEnd() {
    return timeEnd;
  }

  public void setTimeEnd(Calendar timeEnd) {
    this.timeEnd = timeEnd;
  }

  public String getAddress() {
    return address;
  }

  public void setAddress(String address) {
    this.address = address;
  }

  public Boolean isTxmode() {
    return txmode;
  }

  public void setTxmode(Boolean txmode) {
    this.txmode = txmode;
  }

  public Set getDays() {
    return days;
  }

  public void setDays(Set days) {
    this.days = days;
  }
}
