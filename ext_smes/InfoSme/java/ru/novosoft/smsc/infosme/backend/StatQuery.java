package ru.novosoft.smsc.infosme.backend;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 30.09.2003
 * Time: 12:47:58
 * To change this template use Options | File Templates.
 */

import java.util.*;

public class StatQuery
{
  private Date fromDate = new Date();
  private Date tillDate = new Date();
  private String taskId = null;

  private boolean fromDateEnabled = true;
  private boolean tillDateEnabled = false;

  public StatQuery()
  {
    Calendar calendar = new GregorianCalendar();
    calendar.setTime(fromDate);
    calendar.set(Calendar.HOUR_OF_DAY, 0);
    calendar.set(Calendar.MINUTE, 0);
    calendar.set(Calendar.SECOND, 0);
    calendar.set(Calendar.MILLISECOND, 0);
    fromDate = calendar.getTime();
  }

  public Date getFromDate()
  {
    return fromDate;
  }

  public void setFromDate(Date fromDate)
  {
    this.fromDate = fromDate;
  }

  public Date getTillDate()
  {
    return tillDate;
  }

  public void setTillDate(Date tillDate)
  {
    this.tillDate = tillDate;
  }

  public String getTaskId()
  {
    return taskId;
  }

  public void setTaskId(String taskId)
  {
    this.taskId = taskId;
  }

  public void setFromDateEnabled(boolean fromDateEnabled)
  {
    this.fromDateEnabled = fromDateEnabled;
  }

  public void setTillDateEnabled(boolean tillDateEnabled)
  {
    this.tillDateEnabled = tillDateEnabled;
  }

  public boolean isFromDateEnabled()
  {
    return fromDateEnabled;
  }

  public boolean isTillDateEnabled()
  {
    return tillDateEnabled;
  }
}
