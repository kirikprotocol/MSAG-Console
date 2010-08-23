package ru.novosoft.smsc.infosme.backend.tables.stat;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 30.09.2003
 * Time: 12:47:58
 * To change this template use Options | File Templates.
 */

import ru.novosoft.smsc.infosme.backend.config.InfoSmeConfig;
import ru.novosoft.smsc.infosme.backend.config.tasks.Task;
import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.Filter;

import java.util.Date;
import java.util.HashMap;

public class StatQuery implements Filter
{

  private boolean active = true;

  private String taskId = null;

  HashMap tasksCache = new HashMap();

  private Date fromDate = null;
  private Date tillDate = null;

  private boolean fromDateEnabled = false;
  private boolean tillDateEnabled = false;

  private String owner;
  private InfoSmeConfig config;

  public void setConfig(InfoSmeConfig config) {
    this.config = config;
  }

  public String getTaskId() {
    return taskId;
  }
  public void setTaskId(String taskId) {
    this.taskId = taskId;
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
  public void setTillDateEnabled(boolean tillDateEnabled) {
    this.tillDateEnabled = tillDateEnabled;
  }

  public boolean isFromDateEnabled() {
    return fromDateEnabled;
  }
  public boolean isTillDateEnabled() {
    return tillDateEnabled;
  }

  public String getOwner() {
    return owner;
  }

  public void setOwner(String owner) {
    this.owner = owner;
  }

  public boolean isEmpty() {
    return false;
  }

  public boolean isActive() {
    return active;
  }

  public void setActive(boolean active) {
    this.active = active;
  }

  public boolean isItemAllowed(DataItem item) {
    String tId = (String)item.getValue("taskId");
    if (taskId != null && !taskId.equals(tId))
      return false;

    if (active && !config.containsTaskWithId(tId))
      return false;


    if (owner != null && (taskId == null || active)) {      
      String o = (String)tasksCache.get(tId);
      if (o == null) {
        Task t = config.getTask(tId);
        if(t == null) {
          return false;
        }
        o = t.getOwner();
        tasksCache.put(tId, o);
      }
      if (!owner.equals(o))
        return false;
    }

    return true;
  }
}
