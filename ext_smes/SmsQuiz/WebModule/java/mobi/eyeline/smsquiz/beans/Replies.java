package mobi.eyeline.smsquiz.beans;

import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.EmptyFilter;
import ru.novosoft.smsc.jsp.util.helper.statictable.PagedStaticTableHelper;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.jsp.JspWriter;
import java.util.*;
import java.io.*;
import java.text.SimpleDateFormat;
import java.text.ParseException;

import mobi.eyeline.smsquiz.replystats.*;
import mobi.eyeline.smsquiz.QuizBuilder;
import mobi.eyeline.smsquiz.QuizesDataSource;
import mobi.eyeline.smsquiz.quizes.view.QuizQuery;
import mobi.eyeline.smsquiz.quizes.view.QuizData;

/**
 * author: alkhal
 * Date: 05.11.2008
 */
public class Replies extends SmsQuizBean {

  public static final int RESULT_EXPORT_ALL = PRIVATE_RESULT + 1;

  private int pageSize = 0;
  private String mbExportAll = null;
  private String mbQuery = null;
  private QueryResultSet replies = null;
  private ReplyFilter replyFilter = new ReplyFilter();
  private HashMap quizMap = new HashMap();

  private ReplyDataSource ds;

  private boolean initialized = false;

  private static final String DATE_FORMAT = "dd.MM.yyyy HH:mm:ss";

  private final ReplyTableHelper tableHelper = new ReplyTableHelper("reply_table_helper");

  private String quizId;

  private String quizDir;

  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK) return result;

    if (pageSize == 0) {
      pageSize = getSmsQuizContext().getMessagesPageSize();
    }
    try {
      String replyDir = getSmsQuizContext().getConfig().getString("replystats.statsFile_dir");
      quizDir = getSmsQuizContext().getConfig().getString("quizmanager.dir_quiz");
      ds = new ReplyDataSource(replyDir);
      if (pageSize == 0) {
        pageSize = getSmsQuizContext().getMessagesPageSize();
      }
      int maxReplies = getSmsQuizContext().getMaxRepliesTotalSize();
      if(quizId!=null) {
        File file = new File(quizDir+File.separator+quizId+".xml");
        if(file.exists()) {
          replyFilter.setQuizPath(file.getAbsolutePath());
          QuizData data = QuizBuilder.parseAll(replyFilter.getQuizPath());
          replyFilter.setQuizNumber(data.getDestAddress());
          Date date = getActualStartDate(quizId);
          if(date!=null) {
            replyFilter.setQuizDateBegin(date);
          } else {
            replyFilter.setQuizDateBegin(data.getDateBegin());
          }
          replyFilter.setQuizDateEnd(data.getDateEnd());
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
      tableHelper.setFilter(replyFilter);
      tableHelper.setDs(ds);
      tableHelper.setPageSize(pageSize);
      tableHelper.setMaxRows(maxReplies);

    } catch (Exception e) {
      return error("Can't init data source", e);
    }

    return result;
  }

  public void clean() {
    replyFilter.setDateBeginEnabled(false);
    replyFilter.setDateEndEnabled(false);
    replyFilter.setDateBegin(null);
    replyFilter.setDateEnd(null);
    replyFilter.setAddress(null);
  }


  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if (result != RESULT_OK) return result;

    try {

      tableHelper.processRequest(request);

      if (mbExportAll != null) {
        result = processExportAll();
      }
      if(mbQuery != null) {
        processQuery();
      }
      if (initialized) {
        tableHelper.fillTable();
      }
    } catch (Exception e) {
      logger.error("Process error", e);
      error("Error", e);
    }
    return result;
  }

  private int processQuery() {
    initialized = true;
    mbQuery = null;
    tableHelper.reset();

    return RESULT_OK;
  }

  private int processExportAll() {
    mbExportAll = null;

    return RESULT_EXPORT_ALL;
  }

  public void exportAll(HttpServletResponse response, JspWriter out) {
    response.setContentType("file/csv; filename=replies.csv; charset=windows-1251");
    response.setHeader("Content-Disposition", "attachment; filename=replies.csv");

    try {
      out.clear();
      QueryResultSet messages = ds.query(new ReplyQuery(5000000, replyFilter, ReplyTableHelper.DEFAULT_SORT, 0));

      StringBuffer buffer = new StringBuffer();
      ReplyDataItem res;
      for (Iterator iter = messages.iterator(); iter.hasNext();) {
        res = (ReplyDataItem) iter.next();
        buffer.append(StringEncoderDecoder.encode(convertDateToFileStr((Date) res.getValue(ReplyDataSource.REPLY_DATE))))
            .append(",")
            .append(StringEncoderDecoder.encode((String) res.getValue(ReplyDataSource.MSISDN)))
            .append(",")
            .append(StringEncoderDecoder.encode((String) res.getValue(ReplyDataSource.MESSAGE))).append(System.getProperty("line.separator"));
        out.print(buffer);
        buffer.setLength(0);
      }

      out.flush();
    } catch (IOException e) {
      e.printStackTrace();
    }
  }

  public String convertDateToFileStr(Date date) {
    SimpleDateFormat format = ReplyDataSource.DATE_IN_FILE_FORMAT;
    String res = format.format(date);
    return res.trim().replace(' ', ',');
  }

  private void initQuizes() {
    try {
      quizMap.clear();
      QuizesDataSource ds = QuizesDataSource.getInstance();
      QueryResultSet quizesList = ds.query(new QuizQuery(1000, new EmptyFilter(), QuizesDataSource.QUIZ_NAME, 0));
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

  public String getQuizPath() {
    return quizId;
  }

  public String getAddress() {
    return replyFilter.getAddress();
  }

  public void setAddress(String address) {
    replyFilter.setAddress(address);
  }

  public boolean isInitialized() {
    return initialized;
  }

  public void setInitialized(boolean initialized) {
    this.initialized = initialized;
  }

  public void setPageSize(String pageSize) {
    try {
      this.pageSize = Integer.decode(pageSize).intValue();
    } catch (NumberFormatException e) {
      this.pageSize = 0;
    }
  }

  public String getPageSize() {
    return Integer.toString(pageSize);
  }

  public int getPageSizeInt() {
    return pageSize;
  }


  public String getMbExportAll() {
    return mbExportAll;
  }

  public void setMbExportAll(String mbExportAll) {
    this.mbExportAll = mbExportAll;
  }

  public String getMbQuery() {
    return mbQuery;
  }

  public void setMbQuery(String mbQuery) {
    this.mbQuery = mbQuery;
  }

  public QueryResultSet getReplies() {
    return replies;
  }

  public String getQuizId() {
    return quizId;
  }

  public void setQuizId(String quizId) {
    this.quizId = quizId;
  }

  public boolean isQuizId(String quizId) {
    return (this.quizId != null) && quizId.equals(this.quizId);
  }

  public void setFromDate(String fromDate) {
    if (fromDate != null && fromDate.trim().length() > 0) {
      replyFilter.setDateBeginEnabled(true);
      replyFilter.setDateBegin(convertStringToDate(fromDate));
    } else {
      replyFilter.setDateBeginEnabled(false);
    }
  }

  public String getFromDate() {
    return (replyFilter.isDateBeginEnabled()) ? convertDateToString(replyFilter.getDateBegin()) : "";
  }

  public void setTillDate(String tillDate) {
    if (tillDate != null && tillDate.trim().length() > 0) {
      replyFilter.setDateEndEnabled(true);
      replyFilter.setDateEnd(convertStringToDate(tillDate));
    } else {
      replyFilter.setDateEndEnabled(false);
    }
  }

  private Date convertStringToDate(String date) {
    Date converted = new Date();
    try {
      SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
      converted = formatter.parse(date);
    } catch (ParseException e) {
      e.printStackTrace();
      logger.error(e,e);
    }
    return converted;
  }

  public String getTillDate() {
    return (replyFilter.isDateEndEnabled()) ? convertDateToString(replyFilter.getDateEnd()) : "";
  }

  public String convertDateToString(Date date) {
    SimpleDateFormat format = new SimpleDateFormat(DATE_FORMAT);
    return format.format(date);
  }

  public PagedStaticTableHelper getTableHelper() {
    return tableHelper;
  }
}
