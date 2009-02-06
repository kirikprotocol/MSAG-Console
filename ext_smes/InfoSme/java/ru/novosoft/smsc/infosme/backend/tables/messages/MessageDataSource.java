package ru.novosoft.smsc.infosme.backend.tables.messages;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.infosme.backend.Message;
import ru.novosoft.smsc.jsp.util.tables.EmptyResultSet;
import ru.novosoft.smsc.jsp.util.tables.Query;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSource;
import ru.novosoft.smsc.util.AdvancedStringTokenizer;
import ru.novosoft.smsc.util.RandomAccessFileReader;
import ru.novosoft.smsc.util.config.Config;

import java.io.*;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * User: artem
 * Date: 29.05.2008
 */

public class MessageDataSource extends AbstractDataSource {

  protected static final Category log = Category.getInstance(MessageDataSource.class);

  private static String MSG_DATE_FORMAT = "yyMMddHHmmss";
  private static String DIR_DATE_FORMAT = "yyMMdd";
  private static String FILE_DATE_FORMAT = "HH";

  protected final String storeDir;

  public static final String STATE = "state";
  public static final String DATE = "date";
  public static final String MSISDN = "msisdn";
  public static final String REGION = "region";
  public static final String MESSAGE ="message";
  public static final String ID = "id";
  public static final String TASK_ID = "taskId";

  public MessageDataSource(String storeDir) {
    super(new String[]{STATE, DATE, MSISDN, REGION, MESSAGE});
    this.storeDir = storeDir;
  }

  public QueryResultSet query(Query query_to_run) {

    MessageFilter filter = (MessageFilter)query_to_run.getFilter();
    final String deletedStateStr = String.valueOf(Message.State.DELETED.getId());
    final String filterStateStr = (filter.getStatus() == null || filter.getStatus() == Message.State.UNDEFINED) ? null : String.valueOf(filter.getStatus().getId());
    final String filterAbonentStr = filter.getAddress() == null || filter.getAddress().length()==0 ? null : filter.getAddress();

    // Set empty filter because we make filtering here, in data source (for performance purposes)
    init(new MessageQuery(query_to_run.getExpectedResultsQuantity(), null, query_to_run.getSortOrder(), query_to_run.getStartPosition()));

    try {

      final SimpleDateFormat fileNameFormat = new SimpleDateFormat(DIR_DATE_FORMAT + '/' + FILE_DATE_FORMAT);
      final SimpleDateFormat msgDateFormat = new SimpleDateFormat(MSG_DATE_FORMAT);

      List files = getFiles(filter.getTaskId(), filter.getFromDate(), filter.getTillDate());
      if (files.isEmpty())
        return new EmptyResultSet();

      String fromDateString = filter.getFromDate() == null ? null : msgDateFormat.format(filter.getFromDate());
      String tillDateString = filter.getTillDate() == null ? null : msgDateFormat.format(filter.getTillDate());

      String encoding = System.getProperty("file.encoding");

      for (Iterator iter = files.iterator(); iter.hasNext();) {
        File file = (File)iter.next();

        long idbase = getIdBase(fileNameFormat.parse(file.getParentFile().getName() + '/' + file.getName()));

        RandomAccessFile f = null;
        if (log.isDebugEnabled())
          log.debug("Start reading messages from file: " + file.getName());

        try {
          f = new RandomAccessFile(file, "r");

          RandomAccessFileReader is = new RandomAccessFileReader(f);

          String line = is.readLine(encoding); // Skip first string

          while(true) {
            long offset =is.getFilePointer();
            line = is.readLine(encoding);
            if (line == null)
              break;

            int i = line.indexOf(',');
            String stateStr = line.substring(0,i);
            if (stateStr.equals(deletedStateStr) || (filterStateStr != null && !stateStr.equals(filterStateStr)))
              continue;
            int state = Integer.parseInt(stateStr);

            int k = i + 1;
            i = line.indexOf(',', k);

            String dateStr = line.substring(k, i);
            if ((fromDateString != null && fromDateString.compareTo(dateStr) > 0) || (tillDateString != null && tillDateString.compareTo(dateStr) < 0))
              continue;

            k = i+1;
            i = line.indexOf(',', k);
            String msisdn = line.substring(k, i);
            if (filterAbonentStr != null && !filterAbonentStr.equals(msisdn))
              continue;

            k = i+1;
            i = line.indexOf(',', k);
            String region = line.substring(k, i);

            String message = prepareMessage(line.substring(i+2, line.length() - 1));

            long id = getId(idbase, offset);

            add(new MessageDataItem(id, filter.getTaskId(), state, msgDateFormat.parse(dateStr), msisdn, region, message));

          }
        } catch (EOFException e) {
        } catch (IOException e) {
          e.printStackTrace();
        } catch (AdminException e) {
          e.printStackTrace();
        } finally {
          if (f != null)
            try {
              f.close();
            } catch (IOException e) {
            }
        }
      }

    } catch (ParseException e) {
      e.printStackTrace();
    }

    return getResults();
  }

  private static long getId(long idbase, long offset) {
    return idbase | ((int)offset);
  }

  private static long getIdBase(Date date) {
    Calendar cal = Calendar.getInstance();
    cal.setTime(date);
    long y1 = cal.get(Calendar.YEAR) % 10;
    long y2 = (cal.get(Calendar.YEAR) % 100) / 10;
    long m1 = (cal.get(Calendar.MONTH) + 1) % 10;
    long m2 = (cal.get(Calendar.MONTH) + 1) / 10;
    long d1 = cal.get(Calendar.DAY_OF_MONTH) % 10;
    long d2 = cal.get(Calendar.DAY_OF_MONTH) / 10;
    long h1 = cal.get(Calendar.HOUR_OF_DAY) % 10;
    long h2 = cal.get(Calendar.HOUR_OF_DAY) / 10;

    return y2 << 60 | y1 << 56 | m2 << 52 | m1 << 48 | d2 << 44 | d1 << 40 | h2 << 36 | h1 << 32;
  }

  private static String prepareMessage(String m) {
    String message = m;
    message = message.trim();
    message = message.replaceAll("\\\\n", "\n");
    message = message.replaceAll("\\\\\"", "\"");

    return message;
  }

  public long getMessageId(String msisdn, String taskId) {

    String msisdn2 = (msisdn.startsWith("+7")) ? "8" + msisdn.substring(2) : "+7" + msisdn.substring(1);

    String deletedStateStr = String.valueOf(Message.State.DELETED.getId());
    final SimpleDateFormat fileNameFormat = new SimpleDateFormat(DIR_DATE_FORMAT + '/' + FILE_DATE_FORMAT);

    String encoding = System.getProperty("file.encoding");

    List files;
    try {
      files = getFiles(taskId, null, new Date());
    } catch (ParseException e) {
      e.printStackTrace();
      return -1;
    }

    for(Iterator iter = files.iterator(); iter.hasNext();) {
      File file = (File)iter.next();
      RandomAccessFile f = null;

      try {
        f = new RandomAccessFile(file, "r");
        RandomAccessFileReader is = new RandomAccessFileReader(f);

        String line = is.readLine(encoding); // Skip header

        long idbase = getIdBase(fileNameFormat.parse(file.getParentFile().getName() + '/' + file.getName()));

        int i,k;
        while(true) {
          long offset = is.getFilePointer();
          line = is.readLine(encoding);
          if (line == null)
            break;

          // Read state
          i = line.indexOf(',');
          String stateStr = line.substring(0,i);
          if (stateStr.equals(deletedStateStr))
            continue;

          // Skip date
          i = line.indexOf(',', i + 1);

          // Read msisdn
          k = i + 1;
          i = line.indexOf(',', k);
          String ms = line.substring(k, i);
          if (ms.equals(msisdn) || ms.equals(msisdn2))
            return getId(idbase, offset);
        }

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
    }

    return -1;
  }

  private List getFiles (String taskId, Date from, Date till) throws ParseException {

    List files = new LinkedList();

    File dir = new File(storeDir + File.separator + taskId);
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

}
