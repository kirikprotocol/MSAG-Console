package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.infosme.backend.Message;
import ru.novosoft.smsc.infosme.backend.InfoSmeTransport;
import ru.novosoft.smsc.infosme.backend.tables.messages.MessageFilter;
import ru.novosoft.smsc.infosme.backend.tables.messages.MessageDataSource;
import ru.novosoft.smsc.infosme.backend.tables.messages.MessageQuery;
import ru.novosoft.smsc.infosme.backend.tables.messages.MessageDataItem;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskDataSource;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.SortedList;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.jsp.JspWriter;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;
import java.io.IOException;

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
  public static final int RESULT_UPDATE_ALL = PRIVATE_RESULT + 1;
  public static final int RESULT_UPDATE = PRIVATE_RESULT + 2;
  public static final int RESULT_CANCEL_UPDATE = PRIVATE_RESULT + 3;
  public static final int RESULT_EXPORT_ALL = PRIVATE_RESULT + 4;

  private String sort = null;
  private String[] checked = new String[0];
  private Collection checkedSet = null;
  private int startPosition = 0;
  private int pageSize = 0;

  private MessageFilter  msgFilter = new MessageFilter();
  private String message2update;
  private QueryResultSet messages = null;

  private String mbQuery     = null;
  private String mbResend    = null;
  private String mbDelete    = null;

  private String mbUpdate;
  private String mbCancelUpdate;

  private String mbUpdateAll = null;
  private String mbResendAll = null;
  private String mbDeleteAll = null;
  private String mbExportAll = null;

  private StringBuffer exportFile;

  private boolean initialized = false;

  private boolean processed = false;

  private MessageDataSource ds;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK) return result;

    checkedSet = new HashSet(Arrays.asList(checked));

    if (sort != null)  getInfoSmeContext().setMessagesSort(sort);
    else sort = getInfoSmeContext().getMessagesSort();
    if (pageSize != 0) getInfoSmeContext().setMessagesPageSize(pageSize);
    else pageSize = getInfoSmeContext().getMessagesPageSize();

    if (!initialized) {
      msgFilter.setFromDate(Functions.truncateTime(new Date()));
      msgFilter.setFromDateEnabled(true);
    }

    try {
        String serviceFolder = appContext.getHostsManager().getServiceInfo("InfoSme").getServiceFolder().getAbsolutePath();
        String msgStoreDir = getInfoSmeContext().getConfig().getString("InfoSme.storeLocation"); 
	if( msgStoreDir.length() > 0 && msgStoreDir.charAt(0) != '/' ) 
	  msgStoreDir = serviceFolder + '/' + msgStoreDir;
        ds = new MessageDataSource(msgStoreDir);
      } catch (Exception e) {
        return error("Can't init dataa source", e);
      }

    return result;
  }

  public int process(HttpServletRequest request)
  {
    processed = true;
    int result = super.process(request);
    if (result != RESULT_OK) return result;

    Collection allTasks = getAllTasks();
    if (allTasks == null || allTasks.size() <= 0)
      return warning("infosme.warn.no_task_for_msg");

    try { // Order is important here!
      if (mbDelete != null || mbDeleteAll != null) return processDelete();
      else if (mbResend != null || mbResendAll != null) return processResend();
      else if (mbUpdateAll != null) return processUpdateAll();
      else if (mbExportAll != null) return processExportAll();
      else if (mbUpdate != null) return processUpdate();
      else if (mbCancelUpdate != null) return processCancelUpdate();
      else if (mbQuery != null || (initialized && messages == null)) return processQuery();
    } catch (AdminException e) {
      logger.error("Process error", e);
      error("Error", e);
    }
    return result;
  }

  private int processUpdateAll() {
    mbUpdateAll = null;
    return RESULT_UPDATE_ALL;
  }

  private int processExportAll() throws AdminException {
    mbExportAll = null;

    return RESULT_EXPORT_ALL;
  }

  public void exportAll(HttpServletResponse response, JspWriter out) {
    response.setContentType("file/csv; filename=messages.csv; charset=windows-1251");
//    response.setContentLength(exportFile.length());
    response.setHeader("Content-Disposition", "attachment; filename=messages.csv");

    try {
      out.clear();
//      final InfoSmeTransport.GetMessagesResult result = getInfoSme().getMessages(msgFilter.getTaskId(), msgFilter.getStatus(), msgFilter.getFromDate(), msgFilter.getTillDate(), msgFilter.getAddress(),
//                                   (sort != null && sort.startsWith("-")) ? sort.substring(1) : sort, (sort == null || !sort.startsWith("-")), 5000000);
//      final Collection messages = result.getMessages();

      QueryResultSet messages = ds.query(new MessageQuery(5000000, msgFilter, sort, 0));

      StringBuffer buffer = new StringBuffer();
      MessageDataItem msg;
      for (Iterator iter = messages.iterator(); iter.hasNext();) {

        msg = (MessageDataItem)iter.next();
        buffer.append(StringEncoderDecoder.encode((String)msg.getValue("taskId"))).append(",")
            .append(StringEncoderDecoder.encode((String)msg.getValue("msisdn"))).append(",")
            .append(StringEncoderDecoder.encode(getStateName((Message.State)msg.getValue("state")))).append(",")
            .append(StringEncoderDecoder.encode(convertDateToString((Date)msg.getValue("date")))).append(",")
            .append(StringEncoderDecoder.encode((String)msg.getValue("message"))).append('\n');
        out.print(buffer);
        buffer.setLength(0);
      }

      out.flush();
    } catch (IOException e) {
      e.printStackTrace();
    }
  }

  private int processUpdate() {
    mbUpdate = null;
    try {
      getInfoSme().changeDeliveryTextMessage(msgFilter.getTaskId(), msgFilter.getAddress(), msgFilter.getStatus(), msgFilter.getFromDate(), msgFilter.getTillDate(), message2update);
    } catch (AdminException e) {
      logger.debug("Messages update failed", e);
      return error("infosme.error.ds_msg_update_failed", e);
    }
    return RESULT_UPDATE;
  }

  private int processCancelUpdate() {
    mbCancelUpdate = null;
    return RESULT_CANCEL_UPDATE;
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
    else if (state == Message.State.DELETED)
      return "DELETED";

    return "";
  }


  private int processQuery() throws AdminException {
    if (mbQuery != null) { startPosition = 0; mbQuery = null; }

//    final InfoSmeTransport.GetMessagesResult result = getInfoSme().getMessages(msgFilter.getTaskId(), msgFilter.getStatus(), msgFilter.getFromDate(), msgFilter.getTillDate(), msgFilter.getAddress(),
//                                   (sort != null && sort.startsWith("-")) ? sort.substring(1) : sort, (sort == null || !sort.startsWith("-")), getInfoSmeContext().getMaxMessagesTotalSize()+1);
//
//    messages = result.getMessages();

      messages = ds.query(new MessageQuery(getInfoSmeContext().getMaxMessagesTotalSize()+1, msgFilter, sort, startPosition));

//    if (messages.size() > getInfoSmeContext().getMaxMessagesTotalSize())
//      return _error(new SMSCJspException("Messages size is more than " + getInfoSmeContext().getMaxMessagesTotalSize() + ", show first " + getInfoSmeContext().getMaxMessagesTotalSize() + " messages",
//                    SMSCJspException.ERROR_CLASS_MESSAGE));

//    return message("Total messages count: " + messages.getTotalSize());
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
        getInfoSme().deleteMessages(msgFilter.getTaskId(), id);
        deleted++;
      }
    } catch (Throwable e) {
      logger.debug("Messages delete failed", e);
      return error("infosme.error.ds_msg_del_failed", e);
    }

    return message(deleted + " messages have been deleted");
  }

  private int deleteAll() {
    try {
      getInfoSme().deleteMessages(msgFilter.getTaskId(), msgFilter.getAddress(), msgFilter.getStatus(), msgFilter.getFromDate(), msgFilter.getTillDate());

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
        getInfoSme().resendMessages(msgFilter.getTaskId(), id, Message.State.NEW, new Date());
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
      getInfoSme().resendMessages(msgFilter.getTaskId(), msgFilter.getAddress(), msgFilter.getStatus(), msgFilter.getFromDate(), msgFilter.getTillDate(),
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

  public QueryResultSet getMessages() {
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

  public String getMessage2update() {
    return message2update;
  }

  public void setMessage2update(String message2update) {
    this.message2update = message2update;
  }

  public String getMbUpdate() {
    return mbUpdate;
  }

  public void setMbUpdate(String mbUpdate) {
    this.mbUpdate = mbUpdate;
  }

  public String getMbCancelUpdate() {
    return mbCancelUpdate;
  }

  public void setMbCancelUpdate(String mbCancelUpdate) {
    this.mbCancelUpdate = mbCancelUpdate;
  }

  public String getMbUpdateAll() {
    return mbUpdateAll;
  }

  public void setMbUpdateAll(String mbUpdateAll) {
    this.mbUpdateAll = mbUpdateAll;
  }

  public String getMbExportAll() {
    return mbExportAll;
  }

  public void setMbExportAll(String mbExportAll) {
    this.mbExportAll = mbExportAll;
  }

  public boolean isProcessed() {
    return processed;
  }

//  public byte[] getExportFile() {
//    return exportFile;
//  }
}
