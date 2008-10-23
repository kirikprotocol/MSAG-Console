package ru.novosoft.smsc.infosme.backend.commands;

/**
 * author: alkhal
 * Date: 23.10.2008
 */
public class TaskProps {

  private String dateBegin;
  private String dateEnd;
  private boolean txmode;
  private String timeBegin;
  private String timeEnd;
  private String days;
  private String address;


  public String getDateBegin() {
    return dateBegin;
  }

  public void setDateBegin(String dateBegin) {
    this.dateBegin = dateBegin;
  }

  public String getDateEnd() {
    return dateEnd;
  }

  public void setDateEnd(String dateEnd) {
    this.dateEnd = dateEnd;
  }

  public boolean isTxmode() {
    return txmode;
  }

  public void setTxmode(boolean txmode) {
    this.txmode = txmode;
  }

  public String getTimeBegin() {
    return timeBegin;
  }

  public void setTimeBegin(String timeBegin) {
    this.timeBegin = timeBegin;
  }

  public String getTimeEnd() {
    return timeEnd;
  }

  public void setTimeEnd(String timeEnd) {
    this.timeEnd = timeEnd;
  }

  public String getDays() {
    return days;
  }

  public void setDays(String days) {
    this.days = days;
  }

  public String getAddress() {
    return address;
  }

  public void setAddress(String address) {
    this.address = address;
  }

  public boolean validateNull() {
    return !((dateBegin == null) || (dateEnd == null) || (timeBegin == null) || (timeEnd == null)
        || (days == null) || (address == null));
  }
}
