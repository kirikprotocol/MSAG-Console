package ru.novosoft.smsc.infosme.backend;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 30.09.2003
 * Time: 12:47:58
 * To change this template use Options | File Templates.
 */
import java.util.Date;

public class StatQuery
{
  private Date    fromDate = new Date();
  private Date    tillDate = new Date();
  private String  taskId = null;

  private boolean fromDateEnabled = true;
  private boolean tillDateEnabled = false;

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
  public String getTaskId() {
    return taskId;
  }
  public void setTaskId(String taskId) {
    this.taskId = taskId;
  }

  public void setFromDateEnabled(boolean fromDateEnabled) {
      this.fromDateEnabled = fromDateEnabled;
  }
  public void setTillDateEnabled(boolean tillDateEnabled) {
      this.tillDateEnabled = tillDateEnabled;
  }
  public boolean isFromDateEnabled() {
      return fromDateEnabled;
  }
  public boolean isTillDateEnabled() {
      return tillDateEnabled;
  }
}
