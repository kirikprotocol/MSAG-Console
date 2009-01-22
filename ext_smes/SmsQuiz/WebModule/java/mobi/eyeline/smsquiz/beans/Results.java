package mobi.eyeline.smsquiz.beans;


import mobi.eyeline.smsquiz.results.*;
import mobi.eyeline.smsquiz.quizes.view.QuizesDataSource;
import mobi.eyeline.smsquiz.quizes.view.QuizQuery;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.jsp.JspWriter;
import java.util.*;
import java.io.IOException;
import java.io.File;
import java.io.FilenameFilter;
import java.text.SimpleDateFormat;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.EmptyFilter;
import ru.novosoft.smsc.jsp.util.helper.statictable.PagedStaticTableHelper;
import ru.novosoft.smsc.util.StringEncoderDecoder;

/**
 * author: alkhal
 * Date: 05.11.2008
 */
public class Results extends SmsQuizBean {

  public static final int RESULT_EXPORT_ALL = PRIVATE_RESULT + 1;

  private int pageSize = 0;
  private String mbExportAll = null;
  private String mbQuery = null;
  private ResultFilter resultFilter = new ResultFilter();

  private ResultDataSource ds;

  private boolean initialized = false;

  private final ResultTableHelper tableHelper = new ResultTableHelper("result_table_helper");

  Map quizesMap = new HashMap();

  private String quizId;

  public void clean() {
    resultFilter.setAddress(null);
  }

  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK) return result;

    if (pageSize == 0) {
      pageSize = getSmsQuizContext().getMessagesPageSize();
    }

    try {
      String resultDir = getSmsQuizContext().getConfig().getString("quizmanager.dir_result");
      String quizDir = getSmsQuizContext().getConfig().getString("quizmanager.dir_quiz");
      int maxResults = getSmsQuizContext().getMaxResultsTotalSize();
      makeQuizMap(quizDir);
      if(quizId!=null) {
        if(new File(quizDir+File.separator+quizId+".xml").exists()) {
          resultFilter.setQuizId(quizId);
        } else {
          quizId = null;
          initialized = false;
          tableHelper.reset();
        }
      }
      ds = new ResultDataSource(resultDir);
      tableHelper.setPageSize(pageSize);
      tableHelper.setFilter(resultFilter);
      tableHelper.setDs(ds);
      tableHelper.setMaxRows(maxResults);
    } catch (Exception e) {
      return error("Can't init data source", e);
    }

    return result;
  }

  private void makeQuizMap(String quizDir) {
    QuizesDataSource ds = new QuizesDataSource(quizDir);
    QueryResultSet quizesList = ds.query(new QuizQuery(1000, new EmptyFilter(), QuizesDataSource.QUIZ_NAME, 0));
    quizesMap.clear();
    for (int i = 0; i < quizesList.size(); i++) {
      DataItem item = quizesList.get(i);
      String quizName = (String) item.getValue(QuizesDataSource.QUIZ_NAME);
      String quizId = (String)item.getValue(QuizesDataSource.QUIZ_ID);
      quizesMap.put(quizId,quizName);
    }
  }

  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if (result != RESULT_OK) return result;


    try {
      tableHelper.processRequest(request);
      if (mbExportAll != null) {
        result = processExportAll();
      }
      if(mbQuery !=null ) {
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
    response.setContentType("file/csv; filename=results.csv; charset=windows-1251");
    response.setHeader("Content-Disposition", "attachment; filename=results.csv");

    try {
      out.clear();
      QueryResultSet messages = ds.query(new ResultQuery(5000000, resultFilter, ResultTableHelper.DEFAULT_SORT, 0));

      StringBuffer buffer = new StringBuffer();
      ResultDataItem res;
      for (Iterator iter = messages.iterator(); iter.hasNext();) {
        res = (ResultDataItem) iter.next();
        buffer.append(StringEncoderDecoder.encode((String) res.getValue(ResultDataSource.MSISDN))).append(",")
            .append(StringEncoderDecoder.encode(convertDateToString((Date) res.getValue(ResultDataSource.DELIVERY_DATE)))).append(",")
            .append(StringEncoderDecoder.encode(convertDateToString((Date) res.getValue(ResultDataSource.REPLY_DATE)))).append(",")
            .append(StringEncoderDecoder.encode((String) res.getValue(ResultDataSource.CATEGORY))).append(",")
            .append(StringEncoderDecoder.encode((String) res.getValue(ResultDataSource.MESSAGE))).append(System.getProperty("line.separator"));
        out.print(buffer);
        buffer.setLength(0);
      }

      out.flush();
    } catch (IOException e) {
      e.printStackTrace();
    }
  }

  public String convertDateToString(Date date) {
    SimpleDateFormat format = ResultDataSource.DATE_IN_FILE_FORMAT;
    return format.format(date);
  }

  public Map getAllQuizes() {
    return quizesMap;
  }

  public boolean isQuizId(String quizId) {
    return (this.quizId != null) && this.quizId.equals(quizId);
  }

  public String getAddress() {
    return resultFilter.getAddress();
  }

  public void setAddress(String address) {
    resultFilter.setAddress(address);
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

  public void setQuizId(String quizId) {
    this.quizId = quizId;
  }

  public String getQuizId() {
    return quizId;
  }

  public PagedStaticTableHelper getTableHelper() {
    return tableHelper;
  }
}
