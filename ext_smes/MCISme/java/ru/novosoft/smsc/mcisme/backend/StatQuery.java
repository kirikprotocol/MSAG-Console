package ru.novosoft.smsc.mcisme.backend;

import ru.novosoft.smsc.util.Functions;

import java.util.Date;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 02.04.2004
 * Time: 18:07:38
 * To change this template use Options | File Templates.
 */
public class StatQuery
{
  private Date fromDate = new Date();
  private Date tillDate = new Date();

  private boolean fromDateEnabled = true;
  private boolean tillDateEnabled = false;

  public StatQuery() {
    fromDate = Functions.truncateTime(fromDate);
  }

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

  public void setFromDateEnabled(boolean fromDateEnabled) {
    this.fromDateEnabled = fromDateEnabled;
  }
  public boolean isFromDateEnabled() {
    return fromDateEnabled;
  }

  public void setTillDateEnabled(boolean tillDateEnabled) {
    this.tillDateEnabled = tillDateEnabled;
  }
  public boolean isTillDateEnabled() {
    return tillDateEnabled;
  }

}
