package ru.novosoft.smsc.infosme.backend.tables.messages;

import ru.novosoft.smsc.jsp.util.tables.Filter;
import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.infosme.backend.Message;

import java.util.Date;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 26.05.2004
 * Time: 20:19:09
 * To change this template use Options | File Templates.
 */
public class MessageFilter implements Filter
{
  private Date fromDate = new Date();
  private Date tillDate = new Date();
  private Date archiveDate;
  private String taskId = null;
  private String address = null;
  private Message.State status = Message.State.UNDEFINED;

  private boolean fromDateEnabled = false;
  private boolean tillDateEnabled = false;

  public MessageFilter() {
  }

  public Date getFromDate() {
    return fromDateEnabled ? fromDate : null;
  }

  public void setFromDate(Date fromDate) {
    this.fromDate = fromDate;
  }

  public boolean isFromDateEnabled() {
    return fromDateEnabled;
  }

  public void setFromDateEnabled(boolean fromDateEnabled) {
    this.fromDateEnabled = fromDateEnabled;
  }

  public Date getArchiveDate() {
    return archiveDate;
  }

  public void setArchiveDate(Date archiveDate) {
    this.archiveDate = archiveDate;
  }

  public boolean isTillDateEnabled() {
    return tillDateEnabled;
  }

  public void setTillDateEnabled(boolean tillDateEnabled) {
    this.tillDateEnabled = tillDateEnabled;
  }

  public Date getTillDate() {
    return tillDateEnabled ? tillDate : null;
  }

  public void setTillDate(Date tillDate) {
    this.tillDate = tillDate;
  }

  public String getTaskId() {
    return (taskId == null) ? "":taskId;
  }
  public void setTaskId(String taskId) {
    this.taskId = taskId;
  }

  public String getAddress() {
    return (address == null) ? "":address;
  }
  public void setAddress(String address) {
    this.address = address;
  }

  public Message.State getStatus() {
    return status;
  }
  public void setStatus(byte status) {
    this.status = Message.State.getById(status);
  }

  public boolean isEmpty() {
    return false;
  }

  public boolean isItemAllowed(DataItem item) {
    if (!item.getValue(MessageDataSource.TASK_ID).equals(taskId))
      return false;
    if (address != null && !item.getValue(MessageDataSource.MSISDN).equals(address))
      return false;
    if (status != Message.State.UNDEFINED && !item.getValue(MessageDataSource.STATE).equals(status))
      return false;
    return true;
  }

}
