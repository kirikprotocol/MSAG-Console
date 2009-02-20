package mobi.eyeline.smsquiz.replystats;

import org.apache.log4j.Category;
import ru.novosoft.smsc.jsp.util.tables.EmptyResultSet;
import ru.novosoft.smsc.jsp.util.tables.Query;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSource;
import ru.novosoft.smsc.util.RandomAccessFileReader;

import java.io.*;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * author: alkhal
 * Date: 05.11.2008
 */
public class ReplyDataSource extends AbstractDataSource {

  private static final Category log = Category.getInstance(ReplyDataSource.class);

  public static final SimpleDateFormat DATE_IN_FILE_FORMAT = new SimpleDateFormat("yyyyMMdd HH:mm");
  public static final SimpleDateFormat DIR_FORMAT = new SimpleDateFormat("yyyyMMdd");
  public static final SimpleDateFormat FILE_FORMAT = new SimpleDateFormat("HH");

  private static String DIR_DATE_FORMAT = "yyyyMMdd";
  private static String FILE_DATE_FORMAT = "HH";

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
    init(query_to_run);

    final ReplyFilter filter = (ReplyFilter) query_to_run.getFilter();

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


    try {
      String encoding = System.getProperty("file.encoding");
      for (Iterator iter = getFiles(filter.getQuizNumber(), dateBegin, dateEnd).iterator(); iter.hasNext();) {
        File file = (File)iter.next();
        RandomAccessFile f = null;
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

            String text = st.nextToken().trim();
            while (st.hasMoreTokens()) {
              text += "," + st.nextToken();
            }
            text = prepareText(text);

            final ReplyDataItem di = new ReplyDataItem(replyDate, msisdn, text);
            add(di);

          }
        } catch (EOFException e) {
        }  finally {
          if (f != null)
            try {
              f.close();
            } catch (IOException e) {}
        }
      }
    } catch (Exception e) {
      log.error(e,e);
      return new EmptyResultSet();
    }

    return getResults();
  }

  private List getFiles (String dstAddr, Date from, Date till) throws ParseException {

    List files = new LinkedList();

    File dir = new File(replyDir + File.separator + dstAddr);
    if(dir.exists()) {

      final SimpleDateFormat dirNameFormat = new SimpleDateFormat(DIR_DATE_FORMAT);
      final SimpleDateFormat fileNameFormat = new SimpleDateFormat(DIR_DATE_FORMAT + '/' + FILE_DATE_FORMAT);
      final SimpleDateFormat fileDateFormat = new SimpleDateFormat(DIR_DATE_FORMAT + FILE_DATE_FORMAT);

      final Date fromDir = from == null ? null : dirNameFormat.parse(dirNameFormat.format(from));
      final Date fromFile = from == null ? null : fileDateFormat.parse(fileDateFormat.format(from));
      final Date tillDir = till == null ? null : dirNameFormat.parse(dirNameFormat.format(till));
      final Date tillFile = till == null ? null : fileDateFormat.parse(fileDateFormat.format(till));

      // Fetch directories
      File[] dirArr = dir.listFiles(new FileFilter(){
        public boolean accept(File file) {
          if (!file.isDirectory())
            return false;
          try {
            Date dirDate = dirNameFormat.parse(file.getName());
            return (fromDir == null || dirDate.compareTo(fromDir) >= 0) && (tillDir == null || dirDate.compareTo(tillDir) <= 0);
          } catch (ParseException e) {
            return false;
          }
        }
      });

      // Fetch files
      for (int i=0;i<dirArr.length;i++) {
        File directory = dirArr[i];
        String dirName = directory.getName();
        File[] fileArr = directory.listFiles();

        for(int j=0;j<fileArr.length;j++) {
          File f = fileArr[j];
          if (!f.isFile())
            continue;
          String name = f.getName();
          if (name.lastIndexOf(".csv") < 0)
            continue;

          Date fileDate = fileNameFormat.parse(dirName + '/' + name.substring(0, 2));
          if ((tillFile == null || fileDate.compareTo(tillFile) <= 0) && (fromFile == null || fileDate.compareTo(fromFile) >= 0))
            files.add(f);
        }
      }
    }
    return files;
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

  private static String prepareText(String m) {
    String text = m;
    text = text.trim();
    text = text.replaceAll("\\\\n", "\n").replaceAll("\\\\r", "\r");
    text = text.replaceAll("\\\\\"", "\"");

    return text;
  }

}

