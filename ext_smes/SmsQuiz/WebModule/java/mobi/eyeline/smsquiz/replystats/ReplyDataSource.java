package mobi.eyeline.smsquiz.replystats;

import org.apache.log4j.Category;

import java.text.SimpleDateFormat;
import java.io.*;
import java.util.*;

import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.Query;
import ru.novosoft.smsc.jsp.util.tables.EmptyFilter;
import ru.novosoft.smsc.jsp.util.tables.EmptyResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.QueryResultSetImpl;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSourceImpl;
import ru.novosoft.smsc.util.RandomAccessFileReader;

/**
 * author: alkhal
 * Date: 05.11.2008
 */
public class ReplyDataSource extends AbstractDataSourceImpl {

  private static final Category log = Category.getInstance(ReplyDataSource.class);

  public static final SimpleDateFormat DATE_IN_FILE_FORMAT = new SimpleDateFormat("yyyyMMdd HH:mm");
  public static final SimpleDateFormat DIR_FORMAT = new SimpleDateFormat("yyyyMMdd");
  public static final SimpleDateFormat FILE_FORMAT = new SimpleDateFormat("HH");

  private final String replyDir;

  public final static String REPLY_DATE = "replyDate";
  public final static String MSISDN = "msisdn";
  public final static String MESSAGE = "message";

  public ReplyDataSource(String replyDir) {
    super(new String[]{REPLY_DATE, MSISDN, MESSAGE});
    this.replyDir = replyDir;
  }

  private Date resetTillDay(Date date) {
    Calendar cal = Calendar.getInstance();
    cal.setTime(date);
    cal.set(Calendar.HOUR_OF_DAY, 0);
    cal.set(Calendar.MINUTE, 0);
    cal.set(Calendar.SECOND, 0);
    cal.set(Calendar.MILLISECOND, 0);
    return cal.getTime();
  }

  public QueryResultSet query(Query query_to_run) {
    clear();

    final ReplyFilter filter = (ReplyFilter) query_to_run.getFilter();

    int total = 0;
    Date dateBegin;
    if ((filter.isDateBeginEnabled()) && (filter.getDateBegin().after(filter.getQuizDateBegin()))) {
      dateBegin = filter.getDateBegin();
    } else {
      dateBegin = filter.getQuizDateBegin();
    }
    Date dateEnd;
    if ((filter.isDateEndEnabled()) && (filter.getDateEnd().before(filter.getQuizDateEnd()))) {
      dateEnd = filter.getDateEnd();
    } else {
      dateEnd = filter.getQuizDateEnd();
    }
    Date tillDay = resetTillDay(dateBegin);
    File dir = new File(replyDir + File.separator + filter.getQuizNumber());
    Date tillHour = resetTillHour(dateBegin);
    if (!dir.exists()) {
      log.error("dir doesn't exist: " + dir.getAbsolutePath());
      System.out.println("ERROR: dir doesn't exist: " + dir.getAbsolutePath());
      return new EmptyResultSet();
    }
    if (log.isDebugEnabled()) {
      log.debug("DateBegin: " + filter.getDateBegin());
      log.debug("DateEnd: " + filter.getDateEnd());
      log.debug("DateBegin till day: " + tillDay);
      log.debug("DateBegin till hour:" + tillHour);
    }
    RandomAccessFile f = null;
    try {
      File[] directories = dir.listFiles();
      for (int i = 0; i < directories.length; i++) {
        if (directories[i].isDirectory()) {
          String dirName = directories[i].getName();
          Date dirDate = DIR_FORMAT.parse(dirName);
          File[] files = directories[i].listFiles();
          for (int s = 0; s < files.length; s++) {
            File file = files[s];
            if (!file.isFile() || (!file.getName().endsWith(".csv"))) {
              continue;
            }
            String name = file.getName();
            Date fileDate = FILE_FORMAT.parse(name.substring(0, name.lastIndexOf(".")));
            Date date = new Date(dirDate.getTime() + fileDate.getTime() + 7 * 60 * 60 * 1000);
            if (log.isDebugEnabled()) {
              log.debug("File name date: " + date);
            }
            if ((date.compareTo(dateEnd) <= 0) && (date.compareTo(tillHour) >= 0)) {
              String encoding = System.getProperty("file.encoding");
              if (log.isDebugEnabled()) {
                log.debug("Start reading results from file: " + file.getName());
              }
              int j = 0;
              try {
                f = new RandomAccessFile(file, "r");
                RandomAccessFileReader is = new RandomAccessFileReader(f);

                String line;
                while (true) {
                  line = is.readLine(encoding);
                  if (line == null)
                    break;

                  StringTokenizer st = new StringTokenizer(line, ",");
                  String dateStr = st.nextToken().trim();
                  String timeStr = st.nextToken().trim();
                  String msisdn = st.nextToken().trim();
                  Date replyDate = DATE_IN_FILE_FORMAT.parse(dateStr + " " + timeStr);
                  if (log.isDebugEnabled()) {
                    log.debug("Reply date: " + replyDate);
                  }
                  String text = st.nextToken().trim();
                  while (st.hasMoreTokens()) {
                    text += "," + st.nextToken();
                  }
                  j++;
                  final ReplyDataItem di = new ReplyDataItem(replyDate, msisdn, text);
                  if (log.isDebugEnabled())
                    log.debug(line);
                  if (filter.isItemAllowed(di)) {
                    if (log.isDebugEnabled())
                      log.debug("allowed");
                    total++;
                    if (total < query_to_run.getExpectedResultsQuantity()) {
                      add(di);
                      total++;
                    }
                  }

                }
              } catch (EOFException e) {
              }
              if (log.isDebugEnabled())
                log.debug(j + " messages have readed from file: " + file.getName());
            }
          }
        }
      }
    } catch (Exception e) {
      log.error(e);
      e.printStackTrace();
      return new EmptyResultSet();
    } finally {
      if (f != null)
        try {
          f.close();
        } catch (IOException e) {
        }
    }
    query_to_run = new ReplyQuery(query_to_run.getExpectedResultsQuantity(),
        new EmptyFilter(), (String) query_to_run.getSortOrder().get(0), query_to_run.getStartPosition());
    return super.query(query_to_run);
  }

  private Date resetTillHour(Date date) {
    Calendar cal = Calendar.getInstance();
    cal.setTime(date);
    cal.set(Calendar.MINUTE, 0);
    cal.set(Calendar.SECOND, 0);
    cal.set(Calendar.MILLISECOND, 0);
    return cal.getTime();
  }

  public String getReplyDir() {
    return replyDir;
  }

}

