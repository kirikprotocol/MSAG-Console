package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.util.SortedList;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskDataSource;
import ru.novosoft.smsc.infosme.backend.tables.messages.MessageFilter;
import ru.novosoft.smsc.infosme.backend.tables.messages.MessageDataSource;
import ru.novosoft.smsc.infosme.backend.tables.messages.MessageQuery;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;

import javax.servlet.http.HttpServletRequest;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 21.05.2004
 * Time: 16:13:58
 * To change this template use Options | File Templates.
 */
public class Messages extends InfoSmeBean
{
  private static final String DATE_FORMAT = "dd.MM.yyyy HH:mm:ss";

  private String sort = null;
  private String[] checked = new String[0];
  private Collection checkedSet = null;
  private int startPosition = 0;
  private int pageSize = 0;

  private MessageDataSource msgSource = null;
  private MessageFilter  msgFilter = new MessageFilter();
  private QueryResultSet messages = null;

  private String mbQuery     = null;
  private String mbResend    = null;
  private String mbDelete    = null;
  private String mbResendAll = null;
  private String mbDeleteAll = null;

  private boolean initialized = false;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK) return result;

    if (msgSource == null) {
      try {
        msgSource = new MessageDataSource(getInfoSmeContext().getDataSource(),
                        getInfoSmeContext().getConfig().getString("InfoSme.tasksTablesPrefix"));
      } catch (Exception e) {
        return error("Init messages DataSource failed", e);
      }
    }

    checkedSet = new HashSet(Arrays.asList(checked));

    if (sort != null)  getInfoSmeContext().setMessagesSort(sort);
    else sort = getInfoSmeContext().getMessagesSort();
    if (pageSize != 0) getInfoSmeContext().setMessagesPageSize(pageSize);
    else pageSize = getInfoSmeContext().getMessagesPageSize();

    if (!initialized) {
      msgFilter.setFromDate(Functions.truncateTime(new Date()));
      msgFilter.setFromDateEnabled(true);
    }

    return result;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK) return result;

    Collection allTasks = getAllTasks();
    if (allTasks == null || allTasks.size() <= 0)
      return warning("No tasks defined. Access to messages denied");

    if (getInfoSmeContext().getDataSource() == null)
      return error("DataSource not initialized");

    if (mbDelete != null || mbDeleteAll != null) return processDelete();
    else if (mbResend != null || mbResendAll != null) return processResend();
    else if (mbQuery != null || (initialized && messages == null)) return processQuery();

    return result;
  }

  private int processQuery()
  {
    if (mbQuery != null) { startPosition = 0; mbQuery = null; }
    messages = null;
    try {
      messages = msgSource.query(new MessageQuery(pageSize, msgFilter, sort, startPosition));
      String error = msgSource.getError();
      if (error != null) throw new Exception(error);
    } catch (Throwable e) {
      logger.debug("Messages query failed", e);
      return error("Messages query failed", e);
    }
    return RESULT_OK;
  }

  private int processDelete()
  {
    int deleted = 0;
    try {
      if (mbDelete != null)         deleted = msgSource.delete(msgFilter.getTaskId(), checked);
      else if (mbDeleteAll != null) deleted = msgSource.delete(msgFilter);
      else throw new Exception("Internal error. Invalid delete method called");
      String error = msgSource.getError();
      if (error != null) throw new Exception(error);
    } catch (Throwable e) {
      logger.debug("Messages delete failed", e);
      return error("Messages delete failed", e);
    }
    resetChecked();
    mbDelete = mbDeleteAll = null;
    int result = processQuery();
    if (result != RESULT_OK) return result;
    return message(""+deleted+" messages deleted");
  }

  private int processResend()
  {
    int resent = 0;
    try {
      if (mbResend != null)         resent = msgSource.resend(msgFilter.getTaskId(), checked);
      else if (mbResendAll != null) resent = msgSource.resend(msgFilter);
      else throw new Exception("Internal error. Invalid resend method called");
      String error = msgSource.getError();
      if (error != null) throw new Exception(error);
    } catch (Throwable e) {
      logger.debug("Messages resend failed", e);
      return error("Messages resend failed", e);
    }
    resetChecked();
    mbResend = mbResendAll = null;
    int result = processQuery();
    if (result != RESULT_OK) return result;
    return message(""+resent+" messages resending");
  }

  private void resetChecked() {
    checked = new String[0];
    checkedSet = new HashSet(Arrays.asList(checked));
  }

  public QueryResultSet getMessages() {
    return messages;
  }
  public int getTotalSize() {
    return (messages == null) ? 0:messages.getTotalSize();
  }
  public int getTotalSizeInt() {
    return getTotalSize();
  }

  public boolean isInitialized() {
    return initialized;
  }
  public void setInitialized(boolean initialized) {
    this.initialized = initialized;
  }

  public String getMbQuery() {
    return mbQuery;
  }
  public void setMbQuery(String mbQuery) {
    this.mbQuery = mbQuery;
  }
  public String getMbResend() {
    return mbResend;
  }
  public void setMbResend(String mbResend) {
    this.mbResend = mbResend;
  }
  public String getMbDelete() {
    return mbDelete;
  }
  public void setMbDelete(String mbDelete) {
    this.mbDelete = mbDelete;
  }
  public String getMbResendAll() {
    return mbResendAll;
  }
  public void setMbResendAll(String mbResendAll) {
    this.mbResendAll = mbResendAll;
  }
  public String getMbDeleteAll() {
    return mbDeleteAll;
  }
  public void setMbDeleteAll(String mbDeleteAll) {
    this.mbDeleteAll = mbDeleteAll;
  }

  private Date convertStringToDate(String date)
  {
    Date converted = new Date();
    try {
      SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
      converted = formatter.parse(date);
    } catch (ParseException e) {
      e.printStackTrace();
    }
    return converted;
  }
  private String convertDateToString(Date date)
  {
    SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
    return formatter.format(date);
  }

  /* -------------------------- IndexBean delegates -------------------------- */
  public String getStartPosition() {
    return String.valueOf(startPosition);
  }
  public int getStartPositionInt() {
    return startPosition;
  }
  public void setStartPosition(String startPosition) {
    try {
      this.startPosition = Integer.decode(startPosition).intValue();
    } catch (NumberFormatException e) {
      this.startPosition = 0;
    }
  }

  public void setPageSize(String pageSize) {
    try {
      this.pageSize = Integer.decode(pageSize).intValue();
    } catch (NumberFormatException e) {
      this.pageSize = 0;
    }
  }
  public String getPageSize() {
    return String.valueOf(pageSize);
  }
  public int getPageSizeInt() {
    return pageSize;
  }

  public String getSort() {
    return sort;
  }
  public void setSort(String sort) {
    this.sort = sort;
  }

  protected Collection getCheckedSet() {
    return checkedSet;
  }
  public boolean isMessageChecked(String messageId) {
    return checkedSet.contains(messageId);
  }
  public String[] getChecked() {
    return checked;
  }
  public void setChecked(String[] checked)
  {
    this.checked = checked;
  }

  /* -------------------------- MessagesQuery delegates -------------------------- */
  public void setFromDate(String fromDate)
  {
    if (fromDate != null && fromDate.trim().length() > 0) {
      msgFilter.setFromDate(convertStringToDate(fromDate));
      msgFilter.setFromDateEnabled(true);
    } else {
      msgFilter.setFromDateEnabled(false);
    }
  }
  public String getFromDate() {
    return (msgFilter.isFromDateEnabled()) ? convertDateToString(msgFilter.getFromDate()) : "";
  }

  public void setTillDate(String tillDate)
  {
    if (tillDate != null && tillDate.trim().length() > 0) {
      msgFilter.setTillDate(convertStringToDate(tillDate));
      msgFilter.setTillDateEnabled(true);
    } else {
      msgFilter.setTillDateEnabled(false);
    }
  }
  public String getTillDate() {
    return (msgFilter.isTillDateEnabled()) ?  convertDateToString(msgFilter.getTillDate()) : "";
  }

  public String getTaskId() {
    return msgFilter.getTaskId();
  }
  public void setTaskId(String taskId) {
    msgFilter.setTaskId(taskId);
  }
  public boolean isTaskId(String taskId) {
    return msgFilter.getTaskId().equals(taskId);
  }
  public String getTaskName(String taskId)
  {
    try {
      return getConfig().getString(TaskDataSource.TASKS_PREFIX + '.' + StringEncoderDecoder.encodeDot(taskId) + ".name");
    } catch (Throwable e) {
      logger.error("Could not get name for task \"" + taskId + "\"", e);
      error("Could not get name for task \"" + taskId + "\"", e);
      return "";
    }
  }

  public Collection getAllTasks() {
    return new SortedList(getConfig().getSectionChildShortSectionNames(TaskDataSource.TASKS_PREFIX));
  }

  public String getAddress() {
    return msgFilter.getAddress();
  }
  public void setAddress(String address) {
    msgFilter.setAddress(address);
  }

  public int getStatus() {
    return msgFilter.getStatus();
  }
  public void setStatus(int status) {
    msgFilter.setStatus((byte)status);
  }
  public boolean isStatus(int status) {
    return (msgFilter.getStatus() == status);
  }
}
