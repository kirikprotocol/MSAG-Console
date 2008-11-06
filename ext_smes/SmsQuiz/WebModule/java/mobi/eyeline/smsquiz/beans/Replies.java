package mobi.eyeline.smsquiz.beans;

import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.Functions;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.jsp.JspWriter;
import java.util.*;
import java.io.*;
import java.text.SimpleDateFormat;
import java.text.ParseException;

import mobi.eyeline.smsquiz.replystats.*;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.input.SAXBuilder;

/**
 * author: alkhal
 * Date: 05.11.2008
 */
public class Replies extends SmsQuizBean {

  public static final int RESULT_EXPORT_ALL = PRIVATE_RESULT+1;

  private int pageSize;
  private String mbExportAll = null;
  private String mbQuery = null;
  private QueryResultSet replies = null;
  private int startPosition = 0;
  private ReplyFilter replyFilter = new ReplyFilter();
  private String quizDir;
  private HashMap quizMap = new HashMap();

  private ReplyDataSource ds;

  private boolean initialized = false;
  private boolean processed = false;

  private static final String DATE_FORMAT = "dd.MM.yyyy HH:mm:ss";

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK) return result;

    if (pageSize != 0) getSmsQuizContext().setPageSize(pageSize);
    else pageSize = getSmsQuizContext().getPageSize();

    if (!initialized) {
      replyFilter.setDateBegin(Functions.truncateTime(new Date()));
    }
    try {
      String replyDir = getSmsQuizContext().getConfig().getString("replystats.statsFile.dir.name");
      initQuizes();
      ds = new ReplyDataSource(replyDir);
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
      else if (mbQuery != null || (initialized && replies == null)) return processQuery();
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
    replies = ds.query(new ReplyQuery(getSmsQuizContext().getMaxTotalSize()+1, replyFilter,"", startPosition));

    return RESULT_OK;
  }

    private int processExportAll() throws AdminException {
    mbExportAll = null;

    return RESULT_EXPORT_ALL;
  }

  public void exportAll(HttpServletResponse response, JspWriter out) {
    response.setContentType("file/csv; filename=replies.csv; charset=windows-1251");
    response.setHeader("Content-Disposition", "attachment; filename=replies.csv");

    try {
      out.clear();
      QueryResultSet messages = ds.query(new ReplyQuery(5000000, replyFilter,"", 0));

      StringBuffer buffer = new StringBuffer();
      ReplyDataItem res;
      for (Iterator iter = messages.iterator(); iter.hasNext();) {
        res = (ReplyDataItem)iter.next();
        buffer.append(StringEncoderDecoder.encode(convertDateToFileStr((Date)res.getValue("replyDate"))))
            .append(",")
            .append(StringEncoderDecoder.encode((String)res.getValue("msisdn")))
             .append(",")
            .append(StringEncoderDecoder.encode((String)res.getValue("message"))).append(System.getProperty("line.separator"));
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
    String res=format.format(date);
    return res.trim().replace(' ',',');
  }

  private void initQuizes() {
    try {
      quizDir = getSmsQuizContext().getConfig().getString("quizmanager.dir.quiz");
      File dir = new File(quizDir);
      if(!dir.exists()) {
        throw new Exception("Quizes dir doesn't exists: "+quizDir);
      }
      File[] files = dir.listFiles(new FilenameFilter() {
        public boolean accept(File dir, String name) {
          return name.endsWith(".xml") || name.endsWith(".xml.old");
        }
      });
      for(int j=0;j<files.length;j++) {
        String name = files[j].getName();
        name = name.substring(0,name.indexOf("."));
        quizMap.put(name,files[j].getAbsolutePath());
      }
    } catch (Exception e) {
      e.printStackTrace();
      logger.error(e);
    }
  }
  public Collection getAllQuizes() {
    if(quizMap!=null) {
      return quizMap.keySet();
    }
    return null;
  }
  public String getQuizPath(String key){
    if(quizMap!=null) {
      return (String)quizMap.get(key);
    }
    logger.warn("Quiz doesn't exist with path: "+key);
    return null;
  }

  public boolean isQuizId(String quizId) {
      return replyFilter.getQuizNumber().equals(quizId);
  }
  public int getTotalSize() {
    return (replies == null) ? 0 : replies.getTotalSize();
  }
  public int getTotalSizeInt() {
    return getTotalSize();
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
  public QueryResultSet getReplies() {
    return replies;
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


  public String getQuizPath() {
    return replyFilter.getQuizPath();
  }
  public void setQuizPath(String quizPath) {
    replyFilter.setQuizPath(quizPath);
    if(quizPath!=null) {
      XmlData data = parseQuiz(quizPath);
      System.out.println("Parsing completed: address="+data.getAddress()+"," +
          " dateBegin="+data.getDateBegin()+", dateEnd="+data.getDateEnd());
      replyFilter.setQuizNumber(data.getAddress());
      replyFilter.setQuizDateBegin(data.getDateBegin());
      replyFilter.setQuizDateEnd(data.getDateEnd());
    }
  }

  public boolean isQuizPath (String path) {
    return replyFilter.getQuizPath().equals(path);
  }

  public void setFromDate(String fromDate)
  {
    if (fromDate != null && fromDate.trim().length() > 0) {
      replyFilter.setDateBegin(convertStringToDate(fromDate));
    }
  }
  public String getFromDate() {
    return (replyFilter.getDateBegin()!=null) ? convertDateToString(replyFilter.getDateBegin()) : "";
  }

  public void setTillDate(String tillDate)
  {
    if (tillDate != null && tillDate.trim().length() > 0) {
      replyFilter.setDateEnd(convertStringToDate(tillDate));
    }
  }
  private Date convertStringToDate(String date) {
    Date converted = new Date();
    try {
      SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
      converted = formatter.parse(date);
    } catch (ParseException e) {
      e.printStackTrace();
      logger.error(e);
    }
    return converted;
  }
  public String getTillDate() {
    return (replyFilter.getDateEnd()!=null) ?  convertDateToString(replyFilter.getDateEnd()) : "";
  }
  public String convertDateToString(Date date){
    SimpleDateFormat format = new SimpleDateFormat(DATE_FORMAT);
    return format.format(date);
  }

  public XmlData parseQuiz(String path) {
    String address = null;
    Date dateBegin = null;
    Date dateEnd = null;
    SimpleDateFormat dateFormat = new SimpleDateFormat("dd.MM.yyyy HH:mm");

    SAXBuilder sb = new SAXBuilder();
    InputStream stream = null;
    try {
      stream = new FileInputStream(path);
      Document doc = sb.build(stream);
      Element root = doc.getRootElement();
      Element elem;

      if ((elem = root.getChild("replies")) != null) {
        if ((elem = elem.getChild("destination-address")) != null) {
          address = elem.getTextTrim();
        }
        else{
          throw new Exception("Section 'destination-address' doesn't exist in "+path);
        }
      } else {
        throw new Exception("Section 'replies' doesn't exist in "+path);
      }
      if ((elem = root.getChild("general")) != null) {
        Element subElem;
        if ((subElem = elem.getChild("date-begin")) != null) {
          dateBegin = dateFormat.parse(subElem.getTextTrim());
        } else {
          throw new Exception("Section 'date-begin' doesn't exist in "+path);
        }
        if ((subElem = elem.getChild("date-end")) != null) {
          dateEnd = dateFormat.parse(subElem.getTextTrim());
        } else {
          throw new Exception("Section 'date-end' doesn't exist in "+path);
        }
      } else {
        throw new Exception("Section 'general' doesn't exist in "+path);
      }
    } catch (Exception e) {
      logger.error("Parsing exception", e);
      e.printStackTrace();
    } finally {
      if (stream != null) {
        try {
          stream.close();
        } catch (IOException e) {
          logger.error("Error close stream", e);
        }
      }
    }
    return new XmlData(address,dateBegin,dateEnd);
  }
}
