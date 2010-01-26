package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.infosme.backend.Message;
import ru.novosoft.smsc.infosme.backend.tables.messages.*;
import ru.novosoft.smsc.jsp.util.helper.statictable.PagedStaticTableHelper;
import ru.novosoft.smsc.jsp.util.helper.statictable.TableHelperException;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.jsp.JspWriter;
import java.io.IOException;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Collection;
import java.util.Date;
import java.util.List;

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
  public static final int RESULT_EXPORT_ALL_NO_TEXTS = PRIVATE_RESULT + 5;

  private int pageSize = 0;

  private MessageFilter  msgFilter = new MessageFilter();
  private String message2update;

  private String mbQuery     = null;
  private String mbClear     = null;
  private String mbResend    = null;
  private String mbDelete    = null;

  private String mbUpdate;
  private String mbCancelUpdate;

  private String mbUpdateAll = null;
  private String mbResendAll = null;
  private String mbDeleteAll = null;
  private String mbExportAll = null;
  private String mbExportAllNoTexts = null;

  private boolean initialized = false;

  private boolean processed = false;

  private MessageDataSource ds;

  private MessagesTableHelper tableHelper;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK) return result;

    try {
      String serviceFolder = appContext.getHostsManager().getServiceInfo("InfoSme").getServiceFolder().getAbsolutePath();
      String msgStoreDir = getInfoSmeConfig().getStoreLocation();
      if( msgStoreDir.length() > 0 && msgStoreDir.charAt(0) != '/' )
        msgStoreDir = serviceFolder + '/' + msgStoreDir;
      ds = new MessageDataSource(msgStoreDir);
      if(pageSize==0) {
        pageSize = getInfoSmeContext().getMessagesPageSize();
      }

    } catch (Exception e) {
      return error("Can't init data source", e);
    }

    return result;
  }

  public void clean() {
    msgFilter.setTillDate(null);
    msgFilter.setFromDate(null);
    msgFilter.setAddress(null);
    msgFilter.setTillDateEnabled(false);
    msgFilter.setFromDateEnabled(false);
  }

  public int process(HttpServletRequest request)
  {
    processed = true;
    int result = super.process(request);
    if (result != RESULT_OK) return result;

    tableHelper = new MessagesTableHelper("message_table_helper", true, getUser(request));
    int maxTotalSize = getInfoSmeContext().getMaxMessagesTotalSize();
    tableHelper.setFilter(msgFilter);
    tableHelper.setDs(ds);
    tableHelper.setPageSize(pageSize);
    tableHelper.setMaxRows(maxTotalSize);

    Collection allTasks = getAllTasks(request);
    if (allTasks == null || allTasks.size() <= 0)
      return warning("infosme.warn.no_task_for_msg");

    try { // Order is important here!
      tableHelper.processRequest(request);
      if (mbDelete != null || mbDeleteAll != null) result = processDelete(request);
      else if (mbResend != null || mbResendAll != null) result =  processResend(request);
      else if (mbUpdateAll != null) return processUpdateAll();
      else if (mbExportAll != null) return processExportAll();
      else if (mbExportAllNoTexts != null) return processExportAllNoTexts();
      else if (mbUpdate != null) result = processUpdate();
      else if (mbCancelUpdate != null) result = processCancelUpdate();
      else if (mbQuery != null) processQuery();
      else if (mbClear != null) {initialized = false; mbClear = null;}
    } catch (AdminException e) {
      logger.error("Process error", e);
      error("Error", e);
    } catch (TableHelperException e) {
      logger.error("Process error", e);
      error("Error", e);
    }
    if (initialized) {
      try {
        tableHelper.fillTable();
      } catch (TableHelperException e) {
        e.printStackTrace();
        logger.error(e);
        return error(e.getMessage());
      }
    }
    return result;
  }

  private int processQuery() {
    initialized = true;
    tableHelper.reset();
    mbQuery = null;
    return RESULT_OK;
  }

  private int processUpdateAll() {
    mbUpdateAll = null;
    tableHelper.reset();
    return RESULT_UPDATE_ALL;
  }

  private int processExportAll() throws AdminException {
    mbExportAll = null;
    return RESULT_EXPORT_ALL;
  }

  private int processExportAllNoTexts() throws AdminException {
    mbExportAllNoTexts = null;
    return RESULT_EXPORT_ALL_NO_TEXTS;
  }

  public void exportAll(HttpServletResponse response, final JspWriter out, final boolean addTexts) {
    response.setContentType("file/csv; filename=messages.csv; charset=windows-1251");
//    response.setContentLength(exportFile.length());
    response.setHeader("Content-Disposition", "attachment; filename=messages.csv");

    try {
      out.clear();

      ds.visit(new MessageVisitor() {
        public boolean visit(MessageDataItem msg) {
          try {
            out.write(StringEncoderDecoder.encode((String)msg.getValue(MessageDataSource.TASK_ID)));
            out.write(';');
            out.write(StringEncoderDecoder.encode((String)msg.getValue(MessageDataSource.MSISDN)));
            out.write(';');
            out.write(StringEncoderDecoder.encode(getStateName((Message.State)msg.getValue(MessageDataSource.STATE))));
            out.write(';');
            out.write(StringEncoderDecoder.encode(convertDateToString((Date)msg.getValue(MessageDataSource.DATE))));
            if (addTexts) {
              out.write(';');
              out.write(StringEncoderDecoder.encode((String)msg.getValue(MessageDataSource.MESSAGE)));
            }
            out.write('\n');          
          } catch (IOException e) {
            e.printStackTrace();
            return false;
          }
          return true;
        }
      }, msgFilter);

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
    tableHelper.reset();
    return RESULT_UPDATE;
  }

  private int processCancelUpdate() {
    mbCancelUpdate = null;
    return RESULT_CANCEL_UPDATE;
  }

  public static String getStateName(Message.State state) {
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

  private int processDelete(HttpServletRequest request) throws AdminException {

    tableHelper.reset();

    if (mbDelete != null)
      deleteChecked(request);
    else if (mbDeleteAll != null)
      deleteAll();

    mbDelete = mbDeleteAll = null;

    return RESULT_OK;
  }

  private int deleteChecked(HttpServletRequest request) {
    String[] checked = tableHelper.getSelectedMessagesArray(request);
    if(checked==null) {
      return warning("Checked messages list is empty");
    }
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
    return message("Messages have been deleted");
  }

  private int processResend(HttpServletRequest request) throws AdminException {
    if (mbResend != null)
      resendChecked(request);
    else if (mbResendAll != null)
      resendAll();

    mbResend = mbResendAll = null;

    tableHelper.reset();

    return RESULT_OK;
  }

  private int resendChecked(HttpServletRequest request) {
    String[] checked = tableHelper.getSelectedMessagesArray(request);
    if(checked==null) {
      return warning("Checked messages list is empty");
    }
    int resent = 0;
    try {
      for (int i = 0; i < checked.length; i++) {
        System.out.println("Resend message for id:"+checked[i]);
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
  public String getTaskName() {
    return getInfoSmeConfig().getTask(msgFilter.getTaskId()).getName();
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

  public String getMbExportAllNoTexts() {
    return mbExportAllNoTexts;
  }

  public void setMbExportAllNoTexts(String mbExportAllNoTexts) {
    this.mbExportAllNoTexts = mbExportAllNoTexts;
  }

  public String getMbClear() {
    return mbClear;
  }

  public void setMbClear(String mbClear) {
    this.mbClear = mbClear;
  }

  public boolean isProcessed() {
    return processed;
  }

//  public byte[] getExportFile() {
//    return exportFile;
//  }

  public PagedStaticTableHelper getTableHelper() {
    return tableHelper;
  }
}
