package mobi.eyeline.smsquiz.beans;


import mobi.eyeline.smsquiz.results.ResultDataSource;
import mobi.eyeline.smsquiz.results.ResultFilter;
import mobi.eyeline.smsquiz.results.ResultQuery;
import mobi.eyeline.smsquiz.results.ResultDataItem;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.jsp.JspWriter;
import java.util.List;
import java.util.Iterator;
import java.util.Date;
import java.util.Collection;
import java.io.IOException;
import java.text.SimpleDateFormat;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.util.StringEncoderDecoder;

/**
 * author: alkhal
 * Date: 05.11.2008
 */
public class Results extends SmsQuizBean {

  public static final int RESULT_EXPORT_ALL = PRIVATE_RESULT+1;

  private int pageSize;
  private String mbExportAll = null;
  private String mbQuery = null;
  private QueryResultSet results = null;
  private int startPosition = 0;
  private ResultFilter resultFilter = new ResultFilter();

  private ResultDataSource ds;

  private boolean initialized = false;
  private boolean processed = false;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK) return result;

    if (pageSize != 0) getSmsQuizContext().setMessagesPageSize(pageSize);
    else pageSize = getSmsQuizContext().getMessagesPageSize();

    if (!initialized) {
    //todo   filter
    }

    try {
      String resultDir = getSmsQuizContext().getConfig().getString("quizmanager.dir.result");
      ds = new ResultDataSource(resultDir);
    } catch (Exception e) {
      return error("Can't init data source", e);
    }

    return result;
  }


  public int process(HttpServletRequest request) {
    processed = true;
    int result = super.process(request);
    if (result != RESULT_OK) return result;


    try {
      if (mbExportAll != null) return processExportAll();
      else if (mbQuery != null || (initialized && results == null)) return processQuery();
    } catch (AdminException e) {
      logger.error("Process error", e);
      error("Error", e);
    }
    return result;
  }


  private int processQuery() throws AdminException {
    if (mbQuery != null) {
      startPosition = 0;
      mbQuery = null;
    }
    results = ds.query(new ResultQuery(getSmsQuizContext().getMaxMessTotalSize()+1, resultFilter,"", startPosition));

    return RESULT_OK;
  }

    private int processExportAll() throws AdminException {
    mbExportAll = null;

    return RESULT_EXPORT_ALL;
  }

  public void exportAll(HttpServletResponse response, JspWriter out) {
    response.setContentType("file/csv; filename=results.csv; charset=windows-1251");
    response.setHeader("Content-Disposition", "attachment; filename=results.csv");

    try {
      out.clear();
      QueryResultSet messages = ds.query(new ResultQuery(5000000, resultFilter,"", 0));

      StringBuffer buffer = new StringBuffer();
      ResultDataItem res;
      for (Iterator iter = messages.iterator(); iter.hasNext();) {
        res = (ResultDataItem)iter.next();
        buffer.append(StringEncoderDecoder.encode((String)res.getValue("msisdn"))).append(",")
            .append(StringEncoderDecoder.encode(convertDateToString((Date)res.getValue("deliveryDate")))).append(",")
            .append(StringEncoderDecoder.encode(convertDateToString((Date)res.getValue("replyDate")))).append(",")
            .append(StringEncoderDecoder.encode((String)res.getValue("category"))).append(",")
            .append(StringEncoderDecoder.encode((String)res.getValue("message"))).append(System.getProperty("line.separator"));
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

  public Collection getAllQuizes() {
    return ds.getAllQuizes();
  }

  public boolean isQuizId(String quizId) {
      return resultFilter.getQuizId().equals(quizId);
  }
  public int getTotalSize() {
    return (results == null) ? 0 : results.getTotalSize();
  }
  public int getTotalSizeInt() {
    return getTotalSize();
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

  public boolean isProcessed() {
    return processed;
  }

  public void setProcessed(boolean processed) {
    this.processed = processed;
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
  public QueryResultSet getResults() {
    return results;
  }

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

  public void setQuizId(String quizId) {
    resultFilter.setQuizId(quizId);
  }

  public String getQuizId() {
    return resultFilter.getQuizId();
  }
}
