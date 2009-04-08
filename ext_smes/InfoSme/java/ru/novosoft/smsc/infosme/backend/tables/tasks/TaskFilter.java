package ru.novosoft.smsc.infosme.backend.tables.tasks;

import ru.novosoft.smsc.jsp.util.tables.Filter;
import ru.novosoft.smsc.jsp.util.tables.DataItem;

import java.util.Date;

/**
 * User: artem
 * Date: 19.01.2009
 */
public class TaskFilter implements Filter {

  private String name;
  private Date startDate;
  private Date endDate;
  private Boolean active;
  private String owner;


  public TaskFilter(boolean active) {
    this.active = Boolean.valueOf(active);
  }

  public TaskFilter() {
  }

  public void clear() {
    name = null;
    startDate = endDate = null;
    active = null;
    owner = null;
  }

  public boolean isEmpty() {
    return name == null && startDate == null && endDate == null;
  }

  public boolean isItemAllowed(DataItem item) {
    TaskDataItem i = (TaskDataItem) item;
    if (name != null && !i.getName().startsWith(name))
      return false;
    if (startDate != null && i.getEndDate() != null && i.getEndDate().before(startDate))
      return false;
    if (endDate != null && i.getStartDate() != null && i.getStartDate().after(endDate))
      return false;
    if (active != null) {
      Date now = new Date();
      boolean isEventActive = i.isEnabled() && i.getStartDate().before(now) && (i.getEndDate() == null || i.getEndDate().after(now));
      if (isEventActive != active.booleanValue())
        return false;
    }
    if (owner != null && i.getOwner() != null && !i.getOwner().equals(owner))
      return false;

    return true;
  }

  public Date getEndDate() {
    return endDate;
  }

  public void setEndDate(Date endDate) {
    this.endDate = endDate;
  }

  public String getName() {
    return name;
  }

  public void setName(String name) {
    this.name = name;
  }

  public Date getStartDate() {
    return startDate;
  }

  public void setStartDate(Date startDate) {
    this.startDate = startDate;
  }

  public Boolean getActive() {
    return active;
  }

  public void setActive(Boolean active) {
    this.active = active;
  }

  public String getOwner() {
    return owner;
  }

  public void setOwner(String owner) {
    this.owner = owner;
  }
}
