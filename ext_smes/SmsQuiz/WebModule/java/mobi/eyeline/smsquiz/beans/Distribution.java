package mobi.eyeline.smsquiz.beans;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.util.helper.statictable.TableHelperException;
import ru.novosoft.smsc.jsp.util.helper.statictable.PagedStaticTableHelper;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.EmptyFilter;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.infosme.backend.tables.messages.*;
import ru.novosoft.smsc.infosme.backend.Message;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.jsp.JspWriter;
import java.util.*;
import java.io.IOException;
import java.io.File;
import java.text.SimpleDateFormat;
import java.text.ParseException;

import mobi.eyeline.smsquiz.QuizesDataSource;
import mobi.eyeline.smsquiz.quizes.view.QuizQuery;

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

  private boolean initialized = false;

  private HashMap quizMap = new HashMap();

  private MessageDataSource ds;

  private String quizId;

  private MessagesTableHelper tableHelper;

  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK) return result;

    try {
      String quizDir = getSmsQuizContext().getConfig().getString("quizmanager.dir_quiz");
      String msgStoreDir = getConfig().getString("distribution.infosme_stats_dir");
      ds = new MessageDataSource(msgStoreDir);
      int maxTotalSize = getSmsQuizContext().getMaxMessTotalSize();
      if (pageSize == 0) {
        pageSize = getSmsQuizContext().getMessagesPageSize();
      }
      if(quizId!=null) {
        if(new File(quizDir +File.separator+quizId+".xml").exists()) {
          String id = getTaskId(quizId);
          msgFilter.setTaskId(id);
        } else {
          initialized = false;
          tableHelper.reset();
          if(logger.isDebugEnabled()) {
            logger.debug("Trying to refresh quiz with id: "+quizId);
          }
          try{
            QuizesDataSource.getInstance().refreshQuiz(quizId);
          } catch(Exception e) {
            logger.error(e,e);
          }
          result = message("Quiz doesn't exist with id: "+quizId);
          quizId = null;
        }
      }
      initQuizes();
      tableHelper.setFilter(msgFilter);
      tableHelper.setDs(ds);
      tableHelper.setPageSize(pageSize);
      tableHelper.setMaxRows(maxTotalSize);
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

  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if (result != RESULT_OK) return result;
    try{
      tableHelper = new MessagesTableHelper("message_table_helper", false, getUser(request));
      tableHelper.processRequest(request);
      if (mbExportAll != null)
        result = processExportAll();
      if(mbQuery!=null) {
        processQuery();
      }
      if (initialized) {
        tableHelper.fillTable();
      }
      return result;
    }catch(Exception e) {
      logger.error(e,e);
      e.printStackTrace();
      return error(e.getMessage());
    }
  }

  private void initQuizes() {
    try {
      QuizesDataSource ds = QuizesDataSource.getInstance();
      QueryResultSet quizesList = ds.query(new QuizQuery(1000, new EmptyFilter(), QuizesDataSource.QUIZ_NAME, 0));
      quizMap.clear();
      for (int i = 0; i < quizesList.size(); i++) {
        DataItem item = quizesList.get(i);
        String quizName = (String) item.getValue(QuizesDataSource.QUIZ_NAME);
        String quizId = (String)item.getValue(QuizesDataSource.QUIZ_ID);
        quizMap.put(quizId,quizName);
      }
    } catch (Exception e) {
      e.printStackTrace();
      logger.error(e,e);
    }
  }

  public Map getAllQuizes() {
      return quizMap;
  }

  public String getQuizId() {
    return quizId;
  }

  public boolean isQuizId(String quizId) {
    return (this.quizId != null) && quizId.equals(this.quizId);
  }

  private int processExportAll() {
    mbExportAll = null;

    return RESULT_EXPORT_ALL;
  }

  public void exportAll(HttpServletResponse response, JspWriter out) {
    response.setContentType("file/csv; filename=messages.csv; charset=windows-1251");
    response.setHeader("Content-Disposition", "attachment; filename=messages.csv");

    try {
      out.clear();
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
      initialized = true;
      mbQuery = null;
      tableHelper.reset();
      tableHelper.fillTable();
    } catch (TableHelperException e) {
      e.printStackTrace();
      logger.error(e,e);
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

  public void setQuiz(String quizId) {
    msgFilter.setTaskId(quizId);
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

  public PagedStaticTableHelper getTableHelper() {
    return tableHelper;
  }

  public void setQuizId(String quizId) {
    this.quizId = quizId;
  }
}
