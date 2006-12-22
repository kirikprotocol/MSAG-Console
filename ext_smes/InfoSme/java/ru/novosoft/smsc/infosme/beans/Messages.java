package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.infosme.backend.InfoSme;
import ru.novosoft.smsc.infosme.backend.Message;
import ru.novosoft.smsc.infosme.backend.tables.messages.MessageFilter;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskDataSource;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.SortedList;
import ru.novosoft.smsc.util.StringEncoderDecoder;

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

  private MessageFilter  msgFilter = new MessageFilter();
  private List messages = null;
  private InfoSme infoSme = null;

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

    try {
      this.infoSme = new InfoSme(appContext.getHostsManager().getServiceInfo("InfoSme"),
                                 appContext.getConfig().getString("InfoSme.Admin.host"),
                                 appContext.getConfig().getInt("InfoSme.Admin.port"));
    } catch (Exception e) {
      logger.error("Can't init InfoSme", e);
      return RESULT_ERROR;
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
      return warning("infosme.warn.no_task_for_msg");

    try {
      if (mbDelete != null || mbDeleteAll != null) return processDelete();
      else if (mbResend != null || mbResendAll != null) return processResend();
      else if (mbQuery != null || (initialized && messages == null)) return processQuery();
    } catch (AdminException e) {
      logger.error("Process error", e);
      error("Error", e);
    }
    return result;
  }

  public String getStateName(Message.State state) {
    if (state == Message.State.UNDEFINED)
      return "ALL";
    else if (state == Message.State.NEW)
      return "NEW";
    else if (state == Message.State.WAIT)
      return "WAIT";
    else if (state == Message.State.ENROUTE)
      return "ENROUTE";
    else if (state == Message.State.DELIVERED)
      return "DELIVERED";
    else if (state == Message.State.EXPIRED)
      return "EXPIRED";
    else if (state == Message.State.FAILED)
      return "FAILED";

    return "";
  }


  private int processQuery() throws AdminException {
    if (mbQuery != null) { startPosition = 0; mbQuery = null; }
    messages = infoSme.getMessages(msgFilter.getTaskId(), msgFilter.getStatus(), msgFilter.getFromDate(), msgFilter.getTillDate(),
                                   (sort != null && sort.startsWith("-")) ? sort.substring(1) : sort, (sort == null || !sort.startsWith("-")));
    return RESULT_OK;
  }

  private int processDelete() throws AdminException {

    if (mbDelete != null)
      deleteChecked();
    else if (mbDeleteAll != null)
      deleteAll();

    resetChecked();
    mbDelete = mbDeleteAll = null;

    return processQuery();
  }

  private int deleteChecked() {
    int deleted = 0;
    try {
      for (int i = 0; i < checked.length; i++) {
        String id = checked[i];
        infoSme.deleteMessages(msgFilter.getTaskId(), id);
        deleted++;
      }
    } catch (Throwable e) {
      logger.debug("Messages delete failed", e);
      return error("infosme.error.ds_msg_del_failed", e);
    }

    return message(""+deleted+" messages have been deleted");
  }

  private int deleteAll() {
    try {
      infoSme.deleteMessages(msgFilter.getTaskId(), msgFilter.getAddress(), msgFilter.getStatus(), msgFilter.getFromDate(), msgFilter.getTillDate());

    } catch (AdminException e) {
      logger.debug("Messages delete failed", e);
      return error("infosme.error.ds_msg_del_failed", e);
    }
    return message("Messages have been resended");
  }

  private int processResend() throws AdminException {
    if (mbResend != null)
      resendChecked();
    else if (mbResendAll != null)
      resendAll();

    resetChecked();
    mbResend = mbResendAll = null;

    return processQuery();
  }

  private int resendChecked() {
    int resent = 0;
    try {
      for (int i = 0; i < checked.length; i++) {
        String id = checked[i];
        infoSme.resendMessages(msgFilter.getTaskId(), id, Message.State.NEW, new Date());
        resent++;
      }
    } catch (Throwable e) {
      logger.debug("Messages resend failed", e);
      return error("infosme.error.ds_msg_res_failed", e);
    }

    return message(""+resent+" messages resending");
  }

  private int resendAll() {
    try {
      infoSme.resendMessages(msgFilter.getTaskId(), msgFilter.getAddress(), msgFilter.getStatus(), msgFilter.getFromDate(), msgFilter.getTillDate(),
                             Message.State.NEW, new Date());
    } catch (AdminException e) {
      logger.debug("Messages resend failed", e);
      return error("infosme.error.ds_msg_res_failed", e);
    }
    return message("Messages resending");
  }

  private void resetChecked() {
    checked = new String[0];
    checkedSet = new HashSet(Arrays.asList(checked));
  }

  public List getMessages() {
    return messages;
  }
  public int getTotalSize() {
    return (messages == null) ? 0 : messages.size();
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
  public String convertDateToString(Date date)
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
      error("infosme.error.task_name_undefined", taskId, e);
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
    return msgFilter.getStatus().getId();
  }
  public void setStatus(int status) {
    msgFilter.setStatus((byte)status);
  }
  public boolean isStatus(int status) {
    return (msgFilter.getStatus().getId() == status);
  }
}
