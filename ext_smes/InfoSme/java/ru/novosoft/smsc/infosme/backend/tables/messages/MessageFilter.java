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
  private String taskId = null;
  private String address = null;
  private byte status = Message.MESSAGE_UNDEFINED_STATE;

  private boolean fromDateEnabled = false;
  private boolean tillDateEnabled = false;

  public MessageFilter() {
  }

  public Date getFromDate() {
    return fromDate;
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
  public boolean isTillDateEnabled() {
    return tillDateEnabled;
  }
  public void setTillDateEnabled(boolean tillDateEnabled) {
    this.tillDateEnabled = tillDateEnabled;
  }

  public Date getTillDate() {
    return tillDate;
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

  public byte getStatus() {
    return status;
  }
  public void setStatus(byte status) {
    this.status = status;
  }

  public boolean isEmpty() {
    // TODO: ???
    return false;
  }
  public boolean isItemAllowed(DataItem item) {
    // TODO: ???
    return false;
  }

}
