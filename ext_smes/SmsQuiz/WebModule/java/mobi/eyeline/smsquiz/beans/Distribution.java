package mobi.eyeline.smsquiz.beans;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.util.helper.statictable.TableHelperException;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.SortedList;
import ru.novosoft.smsc.infosme.backend.tables.messages.*;
import ru.novosoft.smsc.infosme.backend.Message;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.jsp.JspWriter;
import java.util.*;
import java.io.IOException;
import java.io.File;
import java.io.FilenameFilter;
import java.text.SimpleDateFormat;
import java.text.ParseException;

import mobi.eyeline.smsquiz.distribution.InfoSmeMessagesDataSource;

/**
 * author: alkhal
 * Date: 12.11.2008
 */
public class Distribution extends SmsQuizBean {
  private static final String DATE_FORMAT = "dd.MM.yyyy HH:mm:ss";
  public static final int RESULT_EXPORT_ALL = PRIVATE_RESULT + 4;

  private int startPosition = 0;
  private int pageSize = 0;

  private MessageFilter msgFilter = new MessageFilter();

  private String mbQuery = null;

  private String mbExportAll = null;

  private String quizDir;

  private boolean initialized = false;

  private HashMap quizMap = new HashMap();

  private boolean processed = false;

  private InfoSmeMessagesDataSource ds;

  private MessagesTableHelper tableHelper = new MessagesTableHelper("message_table_helper", false);

  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK) return result;


    try {
      quizDir = getSmsQuizContext().getConfig().getString("quizmanager.dir.quiz");
      initQuizes();
      String msgStoreDir = getConfig().getString("distribution.info.sme.stats.dir");
      String workDir = getConfig().getString("quizmanager.dir.work");
      ds = new InfoSmeMessagesDataSource(msgStoreDir, workDir);
      if (pageSize == 0) {
        pageSize = getSmsQuizContext().getMessagesPageSize();
      }
      int maxTotalSize = getSmsQuizContext().getMaxMessTotalSize();

      tableHelper.setFilter(msgFilter);
      tableHelper.setDs(ds);
      tableHelper.setPageSize(pageSize);
      tableHelper.setMaxTotalSize(maxTotalSize);
    } catch (Exception e) {
      return error("Can't init dataa source", e);
    }

    return result;
  }


  public int process(HttpServletRequest request) {
    processed = true;
    int result = super.process(request);
    if (result != RESULT_OK) return result;

    Collection allQuizes = getAllQuizes();
    if (allQuizes == null || allQuizes.size() <= 0)
      return warning("smsquiz.message.no.quiz");
    if (mbExportAll != null)
      result = processExportAll();
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

  private void initQuizes() {
    try {
      File dir = new File(quizDir);
      if (!dir.exists()) {
        throw new Exception("Quizes dir doesn't exists: " + quizDir);
      }
      File[] files = dir.listFiles(new FilenameFilter() {
        public boolean accept(File dir, String name) {
          return name.endsWith(".xml") || name.endsWith(".xml.old");
        }
      });
      for (int j = 0; j < files.length; j++) {
        String name = files[j].getName();
        name = name.substring(0, name.indexOf("."));
        quizMap.put(name, files[j].getAbsolutePath());
      }
    } catch (Exception e) {
      e.printStackTrace();
      logger.error(e);
    }
  }

  public Collection getAllQuizes() {
    if (quizMap != null) {
      return quizMap.keySet();
    }
    return null;
  }

  public String getQuizPath(String key) {
    if (quizMap != null) {
      return (String) quizMap.get(key);
    }
    logger.warn("Quiz doesn't exist with path: " + key);
    return null;
  }

  private int processExportAll() {
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

      QueryResultSet messages = ds.query(new MessageQuery(5000000, msgFilter, MessagesTableHelper.DEFAULT_SORT, 0));

      StringBuffer buffer = new StringBuffer();
      MessageDataItem msg;
      for (Iterator iter = messages.iterator(); iter.hasNext();) {

        msg = (MessageDataItem) iter.next();
        buffer.append(StringEncoderDecoder.encode((String) msg.getValue(MessageDataSource.MSISDN))).append(",")
            .append(StringEncoderDecoder.encode(getStateName((Message.State) msg.getValue(MessageDataSource.STATE)))).append(",")
            .append(StringEncoderDecoder.encode(convertDateToString((Date) msg.getValue(MessageDataSource.DATE)))).append(",")
            .append(StringEncoderDecoder.encode((String) msg.getValue(MessageDataSource.MESSAGE))).append('\n');
        out.print(buffer);
        buffer.setLength(0);
      }

      out.flush();
    } catch (IOException e) {
      e.printStackTrace();
    }
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
    try {
      tableHelper.fillTable();
    } catch (TableHelperException e) {
      e.printStackTrace();
      logger.error(e);
      throw new AdminException(e.getMessage());
    }
    return RESULT_OK;
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


  private Date convertStringToDate(String date) {
    Date converted = new Date();
    try {
      SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
      converted = formatter.parse(date);
    } catch (ParseException e) {
      e.printStackTrace();
    }
    return converted;
  }

  public String convertDateToString(Date date) {
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


  /* -------------------------- MessagesQuery delegates -------------------------- */
  public void setFromDate(String fromDate) {
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

  public void setTillDate(String tillDate) {
    if (tillDate != null && tillDate.trim().length() > 0) {
      msgFilter.setTillDate(convertStringToDate(tillDate));
      msgFilter.setTillDateEnabled(true);
    } else {
      msgFilter.setTillDateEnabled(false);
    }
  }

  public String getTillDate() {
    return (msgFilter.isTillDateEnabled()) ? convertDateToString(msgFilter.getTillDate()) : "";
  }

  public String getQuiz() {
    return msgFilter.getTaskId();
  }

  public void setQuiz(String taskId) {
    msgFilter.setTaskId(taskId);
  }

  public boolean isQuiz(String quiz) {
    return msgFilter.getTaskId().equals(quiz);
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
    msgFilter.setStatus((byte) status);
  }

  public boolean isStatus(int status) {
    return (msgFilter.getStatus().getId() == status);
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

  public MessagesTableHelper getTableHelper() {
    return tableHelper;
  }

}
