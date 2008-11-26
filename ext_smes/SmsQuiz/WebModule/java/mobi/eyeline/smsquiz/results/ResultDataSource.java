package mobi.eyeline.smsquiz.results;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSourceImpl;
import ru.novosoft.smsc.jsp.util.tables.impl.QueryResultSetImpl;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSource;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.Query;
import ru.novosoft.smsc.jsp.util.tables.EmptyFilter;
import ru.novosoft.smsc.util.RandomAccessFileReader;
import org.apache.log4j.Category;

import java.text.SimpleDateFormat;
import java.text.ParseException;
import java.util.*;
import java.io.*;


/**
 * author: alkhal
 * Date: 05.11.2008
 */
public class ResultDataSource extends AbstractDataSource {

  private static final Category log = Category.getInstance(ResultDataSource.class);

  public static final SimpleDateFormat DATE_IN_FILE_FORMAT = new SimpleDateFormat("dd.MM.yy HH:mm:ss");

  public static final String QUIZ_ID = "quizId";
  public static final String REPLY_DATE = "replyDate";
  public static final String DELIVERY_DATE = "deliveryDate";
  public static final String MSISDN = "msisdn";
  public static final String CATEGORY = "category";
  public static final String MESSAGE = "message";

  private final String resultDir;

  public ResultDataSource(String resultDir) {
    super(new String[]{REPLY_DATE, DELIVERY_DATE, MSISDN, CATEGORY, MESSAGE});
    this.resultDir = resultDir;
  }

  public QueryResultSet query(Query query_to_run) {
    init(query_to_run);
    final ResultFilter filter = (ResultFilter) query_to_run.getFilter();

    File dir = new File(resultDir);
    if (log.isDebugEnabled()) {
      log.debug("resultDir: " + dir.getAbsolutePath());
    }
    System.out.println("QuizId: " + filter.getQuizId());
    File[] files = dir.listFiles(new FilenameFilter() {
      public boolean accept(File dir, String name) {
        return (name.endsWith(".res")) && (name.indexOf(filter.getQuizId()+".") == 0);
      }
    });
    if ((files != null) && (files.length > 0)) {
      File file = files[0];
      System.out.println("Analisys file: " + file.getAbsolutePath());
      String encoding = System.getProperty("file.encoding");
      System.out.println("System file encoding: " + encoding);
      RandomAccessFile f = null;
      if (log.isDebugEnabled()) {
        log.debug("Start reading results from file: " + file.getName());
      }
      try {
        f = new RandomAccessFile(file, "r");

        RandomAccessFileReader is = new RandomAccessFileReader(f);

        String line;
        while (true) {
          line = is.readLine(encoding);
          if (line == null) {
            break;
          }
          StringTokenizer st = new StringTokenizer(line, ",");

          String msisdn = st.nextToken().trim();
          Date deliveryDate = DATE_IN_FILE_FORMAT.parse(st.nextToken().trim());
          Date replyDate = DATE_IN_FILE_FORMAT.parse(st.nextToken().trim());
          String category = st.nextToken();
          String message = st.nextToken();
          while (st.hasMoreTokens()) {
            message += "," + st.nextToken();
          }

          final ResultDataItem di = new ResultDataItem(filter.getQuizId(), replyDate, deliveryDate, msisdn, category, message);
          add(di);
        }
      } catch (EOFException e) {
      } catch (Exception e) {
        log.error(e,e);
        e.printStackTrace();
      } finally {
        if (f != null)
          try {
            f.close();
          } catch (IOException e) {
          }
      }
    }

    return getResults();
  }

  public String getResultDir() {
    return resultDir;
  }

}
