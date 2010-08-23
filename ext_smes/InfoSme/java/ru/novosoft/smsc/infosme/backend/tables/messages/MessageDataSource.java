package ru.novosoft.smsc.infosme.backend.tables.messages;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.infosme.backend.Message;
import ru.novosoft.smsc.infosme.backend.InfoSmeContext;
import ru.novosoft.smsc.infosme.backend.config.tasks.Task;
import ru.novosoft.smsc.infosme.backend.config.tasks.TaskManager;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskArchiveDataSource;
import ru.novosoft.smsc.jsp.util.tables.Query;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSource;
import ru.novosoft.smsc.util.RandomAccessFileReader;

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

  protected final InfoSmeContext infoSmeContext;
  protected TaskArchiveDataSource taskArchiveDataSource;

  public static final String STATE = "state";
  public static final String DATE = "date";
  public static final String MSISDN = "msisdn";
  public static final String REGION = "region";
  public static final String MESSAGE = "message";
  public static final String USERDATA = "userdata";
  public static final String ID = "id";
  public static final String TASK_ID = "taskId";

  public MessageDataSource(InfoSmeContext infoSmeContext) {
    super(new String[]{STATE, DATE, MSISDN, REGION, USERDATA, MESSAGE});
    this.infoSmeContext = infoSmeContext;
  }

  public String getTaskLocation(String taskId, Date archiveDate) throws AdminException {
    if(archiveDate != null) {
      if(taskArchiveDataSource == null) {
        taskArchiveDataSource = new TaskArchiveDataSource(infoSmeContext.getInfoSme(), infoSmeContext.getInfoSmeConfig().getArchiveDir());
      }
      Task t = taskArchiveDataSource.get(archiveDate, taskId);
      return t == null ? null : t.getLocation();
    }else {
      return infoSmeContext.getInfoSmeConfig().getTask(taskId).getLocation();
    }
  }

  public void visit(MessageVisitor visitor, MessageFilter filter) {
    visit(visitor, filter, 1);
  }

  public void visit(MessageVisitor visitor, MessageFilter filter, int threadsNumber) {
    SimpleDateFormat fileNameFormat = new SimpleDateFormat(DIR_DATE_FORMAT + '/' + FILE_DATE_FORMAT);
    SimpleDateFormat msgDateFormat = new SimpleDateFormat(MSG_DATE_FORMAT);

    int filterStatus = (filter.getStatus() == null || filter.getStatus() == Message.State.UNDEFINED) ? -1 : filter.getStatus().getId();
    String fromDateString = filter.getFromDate() == null ? null : msgDateFormat.format(filter.getFromDate());
    String tillDateString = filter.getTillDate() == null ? null : msgDateFormat.format(filter.getTillDate());
    String filterAbonentStr = filter.getAddress() == null || filter.getAddress().length() == 0 ? null : filter.getAddress();

    String encoding = System.getProperty("file.encoding");

    ArrayList threads = new ArrayList(threadsNumber);
    try {
      List files = getFiles(filter.getTaskId(), filter.getFromDate(), filter.getTillDate(), filter.getArchiveDate());
      if (files.isEmpty())
        return;

      int k = 0, linesSize = 1000;
      Line[] lines = new Line[linesSize];
      for (Iterator iter = files.iterator(); iter.hasNext();) {
        File file = (File) iter.next();

        long idbase = getIdBase(fileNameFormat.parse(file.getParentFile().getName() + '/' + file.getName()));

        RandomAccessFile f = null;
        try {
          f = new RandomAccessFile(file, "r");

          RandomAccessFileReader is = new RandomAccessFileReader(f);

          is.readLine(encoding); // Skip first string

          String line;
          long offset;
          while (true) {
            offset = is.getFilePointer();
            if ((line = is.readLine(encoding)) == null)
              break;

            int state = line.charAt(0) - '0';
            if (state == Message.State.DELETED.getId() || (filterStatus != -1 && state != filterStatus))
              continue;

            lines[k++] = new Line(line, getId(idbase, offset));
            if (k >= linesSize) {
              // Offer lines to thread
              parseLines(threads, lines, threadsNumber, visitor, filter, filterAbonentStr, fromDateString, tillDateString);
              lines = new Line[linesSize];
              k = 0;
            }
          }
        } catch (EOFException e) {
        } catch (IOException e) {
          log.error(e,e);
        } finally {
          if (f != null)
            try {
              f.close();
            } catch (IOException e) {
              log.error(e,e);
            }
        }
      }

      // Offer lines to thread
      parseLines(threads, lines, threadsNumber, visitor, filter, filterAbonentStr, fromDateString, tillDateString);

      // Shutdown threads
      for (int i = 0; i < threads.size(); i++) {
        LineParser p = (LineParser) threads.get(i);
        p.shutdown();
        p.join();
      }

    } catch (Throwable e) {
      log.error(e, e);
    } finally {
      for (int i = 0; i < threads.size(); i++) {
        LineParser p = (LineParser) threads.get(i);
        p.shutdown();
      }
    }
  }

  private static void parseLines(ArrayList threads, Line[] lineArray, int threadsNumber, MessageVisitor visitor, MessageFilter filter, String filterAbonentStr, String fromDateStr, String tillDateStr) {
    do {
      for (int i = 0; i < threads.size(); i++) {
        if (((LineParser)threads.get(i)).parseLines(lineArray))
          return;
      }

      if (threads.size() < threadsNumber) {
        LineParser p = new LineParser(visitor, filter, "MessageDataSource-Thread-" + threads.size(), filterAbonentStr, fromDateStr, tillDateStr);
        p.setDaemon(true);
        p.parseLines(lineArray);
        p.start();
        threads.add(p);
        return;
      }

    } while (true);
  }


  public QueryResultSet query(Query query_to_run) {
    init(new MessageQuery(query_to_run.getExpectedResultsQuantity(), null, query_to_run.getSortOrder(), query_to_run.getStartPosition()));

    visit(new MessageVisitor() {
      public boolean visit(MessageDataItem msg) {
        try {
          add(msg);
          return true;
        } catch (AdminException e) {
          log.error(e,e);
          return false;
        }
      }
    }, (MessageFilter) query_to_run.getFilter(), 5);

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
    message = message.replaceAll("\"\"","\"");
    message = message.replaceAll("\\\\n", "\n");
    message = message.replaceAll("\\\\\"", "\""); // for backward compatibility only
    return message;
  }

  public long getMessageId(String msisdn, String taskId) {

    String msisdn2 = (msisdn.startsWith("+7")) ? "8" + msisdn.substring(2) : "+7" + msisdn.substring(1);

    String deletedStateStr = String.valueOf(Message.State.DELETED.getId());
    final SimpleDateFormat fileNameFormat = new SimpleDateFormat(DIR_DATE_FORMAT + '/' + FILE_DATE_FORMAT);

    String encoding = System.getProperty("file.encoding");

    List files;
    try {
      files = getFiles(taskId, null, new Date(), null);
    } catch (Exception e) {
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

  private static boolean isFileContainsMessages(File file) {
    BufferedReader r = null;
    try {
      r = new BufferedReader(new FileReader(file));
      r.readLine(); // Skip header
      String str;
      while ((str = r.readLine()) != null) {
        int state = str.charAt(0) - '0';
        if (state != Message.State.DELETED.getId())
          return true;
      }
    } catch (IOException e) {
      log.error(e, e);
    } finally {
      if (r != null)
        try {
          r.close();
        } catch (IOException e) {
        }
    }
    return false;
  }

  public SortedSet getTaskActivityDates(String taskId, Date taskArchiveDate) throws AdminException {

    File dir = new File(getTaskLocation(taskId, taskArchiveDate));
    TreeSet result = new TreeSet();
    if (dir.exists()) {
      final SimpleDateFormat dirNameFormat = new SimpleDateFormat(DIR_DATE_FORMAT);
      File[] dirArr = dir.listFiles();
      if (dirArr != null) {
        for (int j = 0; j < dirArr.length; j++) {
          if (dirArr[j].isDirectory()) {
            try{
              Date d = dirNameFormat.parse(dirArr[j].getName());
              File[] files = dirArr[j].listFiles();
              if (files != null) {
                for (int i = 0; i < files.length; i++)
                  if (isFileContainsMessages(files[i])) {
                    result.add(d);
                    break;
                  }
              }
            }catch(ParseException e) {}
          }
        }
      }
    }
    return result;
  }

  public boolean isAllMessagesProcessed(String taskId, Date taskArchiveDate) throws AdminException {
    final SimpleDateFormat dirNameFormat = new SimpleDateFormat(DIR_DATE_FORMAT);
    File dir = new File(getTaskLocation(taskId, taskArchiveDate));

    // Fetch directories
    try{
    File[] dirArr = getDirectories(dir, null, null, dirNameFormat);
    if(dirArr == null) {
      return false;
    }
    // Fetch files
    for (int i=0;i<dirArr.length;i++) {
      File directory = dirArr[i];
      File[] fileArr = directory.listFiles();

      for(int j=0;j<fileArr.length;j++) {
        File f = fileArr[j];
        if (!f.isFile())
          continue;
        String name = f.getName();
        if (!name.endsWith(".csv"))
          continue;
        if(!name.endsWith("processed.csv")) {
          return false;
        }
      }
    }
    }catch (ParseException e){
      log.error(e,e);
      throw new AdminException(e.getMessage(), e);
    }
    return true;
  }

  private static File[] getDirectories(File taskDir,  Date from, Date till, final SimpleDateFormat dirNameFormat) throws ParseException {
    final Date fromDir = from == null ? null : dirNameFormat.parse(dirNameFormat.format(from));
    final Date tillDir = till == null ? null : dirNameFormat.parse(dirNameFormat.format(till));
    return taskDir.listFiles(new FileFilter(){
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
  }

  private List getFiles(String taskId, Date from, Date till, Date archiveDate) throws ParseException, AdminException {

    List files = new LinkedList();



    File dir = new File(getTaskLocation(taskId, archiveDate));
    if(dir.exists()) {

      final SimpleDateFormat dirNameFormat = new SimpleDateFormat(DIR_DATE_FORMAT);
      final SimpleDateFormat fileNameFormat = new SimpleDateFormat(DIR_DATE_FORMAT + '/' + FILE_DATE_FORMAT);
      final SimpleDateFormat fileDateFormat = new SimpleDateFormat(DIR_DATE_FORMAT + FILE_DATE_FORMAT);

      final Date fromFile = from == null ? null : fileDateFormat.parse(fileDateFormat.format(from));
      final Date tillFile = till == null ? null : fileDateFormat.parse(fileDateFormat.format(till));

      // Fetch directories
      File[] dirArr = getDirectories(dir, from, till, dirNameFormat);

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

  private static class Line {
    String line;
    long id;

    private Line(String line, long id) {
      this.line = line;
      this.id = id;
    }
  }

  private static class LineParser extends Thread {

    private final Lines lines = new Lines();
    private final MessageVisitor visitor;
    private final MessageFilter filter;
    private final String filterAbonentStr;
    private final String fromDateString;
    private final String tillDateString;

    private LineParser(MessageVisitor visitor, MessageFilter filter, String name, String filterAbonentStr, String fromDateString, String tillDateStr) {
      super(name);
      this.filterAbonentStr = filterAbonentStr;
      this.fromDateString = fromDateString;
      this.tillDateString = tillDateStr;
      this.visitor = visitor;
      this.filter = filter;
    }

    public boolean parseLines(Line[] lines) {
      return this.lines.set(lines);
    }

    public void shutdown() {
      this.lines.shutdown();
    }

    public void run() {
      try {
        final SimpleDateFormat msgDateFormat = new SimpleDateFormat(MSG_DATE_FORMAT);

        while (true) {

          Line[] lineArray = lines.get();
          if (lineArray == null)
            break;

          for (int l = 0; l < lineArray.length; l++) {

            Line lineObj = lineArray[l];
            if (lineObj == null)
              break;

            String line = lineObj.line;

            int state = line.charAt(0) - '0';
            int i = 1;

            int k = i + 1;
            i = line.indexOf(',', k);

            String dateStr = line.substring(k, i);
            if ((fromDateString != null && fromDateString.compareTo(dateStr) > 0) || (tillDateString != null && tillDateString.compareTo(dateStr) < 0))
              continue;

            k = i + 1;
            i = line.indexOf(',', k);
            String msisdn = line.substring(k, i);
            if (filterAbonentStr != null && !filterAbonentStr.equals(msisdn))
              continue;

            k = i + 1;
            i = line.indexOf(',', k);
            String region = line.substring(k, i);

            i += 1;
            char c = line.charAt(i);
            String userData = null;
            String message = null;
            if ( c == ',' ) {
              // userData field is empty
              message = prepareMessage(line.substring(i+2,line.length()-1));
            } else if ( c != '"' ) {
              // user data specified
              k = line.indexOf(',',i);
              userData = line.substring(i,k);
              message = prepareMessage(line.substring(k+2,line.length()-1));
            } else {
              // user data field is not present (old format)
              message = prepareMessage(line.substring(i+1,line.length()-1));
            }

            MessageDataItem item = new MessageDataItem(lineObj.id, filter.getTaskId(), state, msgDateFormat.parse(dateStr), msisdn, region, message, userData);
            if (!visitor.visit(item))
              break;
          }

          lines.clear();
        }

      } catch (Exception e) {
        log.error(e,e);
      } finally {
        lines.clear();
      }
    }
  }

  private static class Lines {

    private Line[] value;
    private boolean shutdowned = false;

    public synchronized boolean set(Line[] list) {
      if (this.value != null)
        return false;
      this.value = list;
      notify();
      return true;
    }

    public synchronized Line[] get() throws InterruptedException {
      if (value == null && !shutdowned)
        wait();
      return value;
    }

    public synchronized void clear() {
      value = null;
    }

    public synchronized void shutdown() {
      shutdowned = true;
      notify();
    }

  }

}
