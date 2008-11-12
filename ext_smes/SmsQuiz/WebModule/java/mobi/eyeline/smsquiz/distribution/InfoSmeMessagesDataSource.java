package mobi.eyeline.smsquiz.distribution;

import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.Query;
import ru.novosoft.smsc.jsp.util.tables.EmptyResultSet;
import ru.novosoft.smsc.jsp.util.tables.EmptyFilter;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSourceImpl;
import ru.novosoft.smsc.infosme.backend.Message;
import ru.novosoft.smsc.infosme.backend.Task;
import ru.novosoft.smsc.infosme.backend.tables.messages.MessageFilter;
import ru.novosoft.smsc.infosme.backend.tables.messages.MessageDataSource;
import ru.novosoft.smsc.infosme.backend.tables.messages.MessageDataItem;
import ru.novosoft.smsc.infosme.backend.tables.messages.MessageQuery;
import ru.novosoft.smsc.util.RandomAccessFileReader;

import java.io.*;
import java.util.*;
import java.text.ParseException;
import java.text.SimpleDateFormat;

import mobi.eyeline.smsquiz.QuizShortData;
import mobi.eyeline.smsquiz.QuizBuilder;
import org.apache.log4j.Category;

/**
 * User: alkhal
 * Date: 12.11.2008
 */

public class InfoSmeMessagesDataSource extends AbstractDataSourceImpl {

  private String workDir;
  protected static final Category log = Category.getInstance(InfoSmeMessagesDataSource.class);

  protected static final SimpleDateFormat dirNameFormat = new SimpleDateFormat("yyMMdd");
  public static final SimpleDateFormat msgDateFormat = new SimpleDateFormat("yyMMddHHmmss");

  protected final String storeDir;

  public InfoSmeMessagesDataSource(String storeDir, String workDir) {
    super(new String[]{MessageDataSource.STATE, MessageDataSource.DATE,
        MessageDataSource.MSISDN, MessageDataSource.REGION, MessageDataSource.MESSAGE});
    this.storeDir = storeDir;
    this.workDir = workDir;
  }

  private String getQuizId(String quizName) {
    String res = null;
    String path = workDir + File.separator + quizName + ".status";
    File file = new File(path);
    if (!file.exists()) {
      path += ".old";
      if (!(file = new File(path)).exists()) {
        log.warn("Status file doen't exist for quiz: " + quizName);
        System.err.println("WARNING: Status file doen't exist for quiz: " + quizName);
        return null;
      }
    }
    InputStream stream = null;
    try {
      stream = new FileInputStream(file);
      Properties prop = new Properties();
      prop.load(stream);
      String id = prop.getProperty("distribution.id");
      if ((id != null) && (!id.trim().equals(""))) {
        res = id;
      } else {
        log.warn("Property id doesn't exist in " + path);
        System.err.println("WARNING: Property id doesn't exist in " + path);
      }
    } catch (IOException e) {
      log.error(e);
      e.printStackTrace();
    } finally {
      if (stream != null) {
        try {
          stream.close();
        } catch (IOException e) {
        }
      }
    }
    return res;
  }

  public QueryResultSet query(Query query_to_run) {
    clear();
    MessageFilter filter = (MessageFilter) query_to_run.getFilter();

    Date fromDate;
    Date tillDate;
    String quizPath = filter.getTaskId();
    File file = new File(quizPath);
    System.out.println("Quiz path: " + quizPath);
    String quizName = file.getName().substring(0, file.getName().indexOf(".xml"));
    String quizId = getQuizId(quizName);

    if (quizId != null) {
      fromDate = filter.getFromDate();
      tillDate = filter.getTillDate();
      if (fromDate == null) {
        try {
          QuizShortData data = QuizBuilder.parseQuiz(quizPath);
          fromDate = data.getDateBegin();
        } catch (Exception e) {
          log.error(e);
          e.printStackTrace();
          return new EmptyResultSet();
        }
      }
      if (tillDate == null)
        tillDate = new Date();

      try {
        fromDate = msgDateFormat.parse(msgDateFormat.format(fromDate));   // to format, where year is yy (not yyyy)
        tillDate = msgDateFormat.parse(msgDateFormat.format(tillDate));
        if (log.isDebugEnabled()) {
          log.debug("FromDate: " + fromDate);
          log.debug("TillDate: " + tillDate);
        }
      } catch (Exception e) {
        log.error(e);
        e.printStackTrace();
        return new EmptyResultSet();

      }

      Calendar cal = Calendar.getInstance();
      cal.setTime(fromDate);

      int total = 0;
      while (cal.getTime().before(tillDate) || cal.getTime().equals(tillDate)) {

        String dirName = dirNameFormat.format(cal.getTime());
        int hour = cal.get(Calendar.HOUR_OF_DAY);

        final String fileNamePrefix;
        if (hour < 10) {
          fileNamePrefix = "0" + String.valueOf(hour);
        } else {
          fileNamePrefix = String.valueOf(hour);
        }

        File dir = new File(storeDir, quizId + '/' + dirName);
        if (log.isDebugEnabled()) {
          log.debug("dir: " + dir.getAbsolutePath());
          log.debug("namePrefix: " + fileNamePrefix);
        }
        File[] files = dir.listFiles(new FilenameFilter() {
          public boolean accept(File dir, String name) {
            return name.equals(fileNamePrefix + "processed.csv") || name.equals(fileNamePrefix + ".csv");
          }
        });

        if (files != null) {
          long y1 = cal.get(Calendar.YEAR) % 10;
          long y2 = (cal.get(Calendar.YEAR) % 100) / 10;
          long m1 = (cal.get(Calendar.MONTH) + 1) % 10;
          long m2 = (cal.get(Calendar.MONTH) + 1) / 10;
          long d1 = cal.get(Calendar.DAY_OF_MONTH) % 10;
          long d2 = cal.get(Calendar.DAY_OF_MONTH) / 10;
          long h1 = cal.get(Calendar.HOUR_OF_DAY) % 10;
          long h2 = cal.get(Calendar.HOUR_OF_DAY) / 10;

          long idbase = y2 << 60 | y1 << 56 | m2 << 52 | m1 << 48 | d2 << 44 | d1 << 40 | h2 << 36 | h1 << 32;
          String encoding = System.getProperty("file.encoding");

          for (int i = 0; i < files.length; i++) {
            file = files[i];

            RandomAccessFile f = null;
            int j = 0;
            if (log.isDebugEnabled())
              log.debug("Start reading messages from file: " + file.getName());
            try {
              f = new RandomAccessFile(file, "r");

              RandomAccessFileReader is = new RandomAccessFileReader(f);

              String line = is.readLine(encoding); // Skip first string

              while (true) {
                long offset = is.getFilePointer();
                line = is.readLine(encoding);
                if (line == null)
                  break;

                j++;

                StringTokenizer st = new StringTokenizer(line, ",");
                int state = Integer.parseInt(st.nextToken().trim());
                if (state == Message.State.DELETED.getId())
                  continue;

                Date date = msgDateFormat.parse(st.nextToken().trim());
                if (!date.after(fromDate) || !date.before(tillDate)) {
                  continue;
                }
                String msisdn = st.nextToken().trim();
                String region = st.nextToken();
                String message = prepareMessage(st.nextToken());

                long id = idbase | ((int) offset);

                final MessageDataItem di = new MessageDataItem(id, filter.getTaskId(), state, date, msisdn, region, message);
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
            } catch (FileNotFoundException e) {
            } catch (EOFException e) {
            } catch (IOException e) {
              e.printStackTrace();
            } catch (ParseException e) {
              e.printStackTrace();
            } finally {
              if (f != null)
                try {
                  f.close();
                } catch (IOException e) {
                }
            }
            if (log.isDebugEnabled())
              log.debug(j + " messages have readed from file: " + file.getName());
          }
        }
        cal.add(Calendar.HOUR_OF_DAY, 1);
      }
    }
    query_to_run = new MessageQuery(query_to_run.getExpectedResultsQuantity(),
        new EmptyFilter(), (String) query_to_run.getSortOrder().get(0), query_to_run.getStartPosition());
    return super.query(query_to_run);
  }

  private static String prepareMessage(String m) {
    String message = m;
    message = message.trim();
    // remove braces
    message = message.substring(1, message.length() - 1);

    message = message.replaceAll("\\\\n", "\n");
    message = message.replaceAll("\\\\\"", "\"");

//    message.replaceAll("\\\\", "\\");

    return message;
  }

}
