package mobi.eyeline.informer.admin.journal;

import com.eyeline.utils.FileUtils;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import org.apache.log4j.Logger;

import java.io.*;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

/**
 * Файловый сторадж журнала
 * Возвращает записи, созданные не более 1 недели назад
 *
 * @author Aleksandr Khalitov
 */
class JournalFileDataSource implements JournalDataSource {

  private final ReadWriteLock lock = new ReentrantReadWriteLock();

  private static final Logger logger = Logger.getLogger(JournalFileDataSource.class);

  private final File journalDir;

  private final FileSystem fileSystem;

  public JournalFileDataSource(File journalDir, FileSystem fileSystem) throws AdminException {
    this.journalDir = journalDir;
    this.fileSystem = fileSystem;
    if (!fileSystem.exists(journalDir)) {
      fileSystem.mkdirs(journalDir);
    }

    try {
      long time = System.currentTimeMillis();
      Collection<File> files = getFiles(null);
      for (File f : files) {
        if (FileUtils.truncateFile(f, System.getProperty("line.separator").getBytes()[0], 30)) {
          if (logger.isInfoEnabled()) {
            logger.info("Journal file was repaired: " + f.getAbsolutePath());
          }
        }
      }
      if (logger.isInfoEnabled()) {
        logger.info("Journal is loaded: " + (System.currentTimeMillis() - time) + " millis");
      }
    } catch (IOException e) {
      logger.error(e, e);
      throw new IllegalArgumentException(e);
    } catch (ParseException e) {
      logger.error(e, e);
      throw new IllegalArgumentException(e);
    }
  }

  @SuppressWarnings({"EmptyCatchBlock"})
  public void addRecords(JournalRecord... records) throws AdminException {
    if (records == null || records.length == 0) {
      return;
    }
    Date date = new Date();
    SimpleDateFormat sdf = new SimpleDateFormat("yyyy" + File.separatorChar + "MM" + File.separator + "dd");
    File d = new File(journalDir, sdf.format(date));
    if (!fileSystem.exists(d)) {
      fileSystem.mkdirs(d);
    }
    File file = new File(d, new SimpleDateFormat("HH").format(date) + ".csv");

    long dateL = date.getTime();

    PrintWriter writer = null;
    try {
      lock.writeLock().lock();
      writer = new PrintWriter(new BufferedWriter(new OutputStreamWriter(fileSystem.getOutputStream(file, true))));
      for (JournalRecord record : records) {
        record.setTime(dateL);
        write(record, writer);
      }
    } finally {
      if (writer != null) {
        try {
          writer.close();
        } catch (Exception ex) {
        }
      }
      lock.writeLock().unlock();
    }
  }

  @SuppressWarnings({"EmptyCatchBlock"})
  public void visit(JournalFilter filter, JournalVisitor v) throws AdminException {
    try {
      Collection<File> files = getFiles(filter);
      try {
        lock.readLock().lock();
        for (File f : files) {
          BufferedReader reader = null;
          try {
            reader = new BufferedReader(new InputStreamReader(fileSystem.getInputStream(f)));
            String line;
            while ((line = reader.readLine()) != null) {
              if (line.length() == 0) {
                continue;                   // may happens in repaired file
              }
              JournalRecord record = convert(line);
              if (!filter.accept(record)) {
                continue;
              }
              if (!v.visit(record)) {
                return;
              }
            }
          } finally {
            if (reader != null) {
              try {
                reader.close();
              } catch (IOException e) {
              }
            }
          }
        }
      } finally {
        lock.readLock().unlock();
      }
    } catch (IOException e) {
      logger.error(e, e);
      throw new JournalException("cant_read_journal", e);
    } catch (ParseException e) {
      logger.error(e, e);
      throw new JournalException("cant_read_journal", e);
    } catch (NumberFormatException e) {
      logger.error(e, e);
      throw new JournalException("cant_read_journal", e);
    }
  }

  private static final char sepChar = '|';
  private static final String sep = new String(new char[]{sepChar});

  private static final String NULL = "null";

  protected static void writeStr(String str, PrintWriter w) {
    if (str == null || str.length() == 0)
      w.write(NULL);
    else {
      for (int i = 0; i < str.length(); i++) {
        char ch = str.charAt(i);
        if (ch == sepChar) {
          w.print('\\');
          w.print(ch);
        }else if(ch == '\n') {
          w.print("\\n");
        }else if(ch == '\r') {
          w.print("\\r");
        }else if(ch == '\t') {
          w.print("\\t");
        }else {
          w.print(ch);
        }
        if (ch == sepChar)
          w.print(' ');
      }
    }
  }

  protected void write(JournalRecord r, PrintWriter w) {
    w.write(Long.toString(r.getTime()));
    w.write(sepChar);
    writeStr(r.getUser(), w);
    w.write(sepChar);
    writeStr(r.getSubject().getKey(), w);
    w.write(sepChar);
    w.write(r.getType() == null ? null : r.getType().toString());
    w.write(sepChar);
    writeStr(r.getDescriptionKey(), w);
    if (r.getDescriptionArgs() != null) {
      for (String a : r.getDescriptionArgs()) {
        w.write(sepChar);
        writeStr(a, w);
      }
    }
    w.println();
  }
  protected static String readFullyStr(String initialStr, StringTokenizer t) {
    int i = 0;
    while ((i = initialStr.indexOf("\\")) != -1) {
      if(initialStr.length() == i+1) {
        initialStr = initialStr.replace('\\', sepChar) + t.nextToken().substring(1);
      }else {
        String old = initialStr;
        initialStr = old.substring(0, i);
        switch (old.charAt(i+1)) {
          case 'n' : initialStr+='\n';break;
          case 't' : initialStr+='\t';break;
          case 'r' : initialStr+='\r';break;
        }
        if(old.length()>i+2) {
          initialStr += old.substring(i+2);
        }
      }
    }
    return initialStr;
  }

  protected static JournalRecord convert(String ss)  {
    StringTokenizer t = new StringTokenizer(ss, sep);
    String sTemp;
    long time = Long.parseLong(t.nextToken());
    String user = (sTemp = t.nextToken()).equals(NULL) ? null : sTemp;
    if (user != null) {
      user = readFullyStr(user, t);
    }
    String subjectKey = (sTemp = t.nextToken()).equals(NULL) ? null : sTemp;
    if (subjectKey != null) {
      subjectKey = readFullyStr(subjectKey, t);
    }
    JournalRecord.Type type = (sTemp = t.nextToken()).equals(NULL) ? null : JournalRecord.Type.valueOf(sTemp);

    String descrKey = (sTemp = t.nextToken()).equals(NULL) ? null : sTemp;
    if (descrKey != null) {
      descrKey = readFullyStr(descrKey, t);
    }

    List<String> args = new LinkedList<String>();
    while (t.hasMoreTokens()) {
      String arg = t.nextToken();
      if (arg != null) {
        arg = readFullyStr(arg, t);
      }
      args.add(arg);
    }
    JournalRecord record = new JournalRecord(type);
    record.setDescription(descrKey, args.toArray(new String[args.size()]));
    record.setSubject(Subject.getByKey(subjectKey));
    record.setUser(user);
    record.setTime(time);
    return record;
  }

  private List<File> getYearsDir(File parent, Date fromDate, Date toDate) throws ParseException {
    List<File> result = new LinkedList<File>();
    SimpleDateFormat ydf = new SimpleDateFormat("yyyy");
    for (File y : parent.listFiles()) {
      Date yd = ydf.parse(y.getName());
      if (toDate != null && yd.after(ydf.parse(ydf.format(toDate)))) {
        continue;
      }
      if (fromDate != null && yd.before(ydf.parse(ydf.format(fromDate)))) {
        continue;
      }
      result.add(y);
    }
    return result;
  }

  private List<File> getMonthsDir(File y, Date fromDate, Date toDate) throws ParseException {
    List<File> result = new LinkedList<File>();
    SimpleDateFormat mdf = new SimpleDateFormat("yyyyMM");
    for (File m : y.listFiles()) {
      Date md = mdf.parse(y.getName() + m.getName());
      if (toDate != null && md.after(mdf.parse(mdf.format(toDate)))) {
        continue;
      }
      if (fromDate != null && md.before(mdf.parse(mdf.format(fromDate)))) {
        continue;
      }
      result.add(m);
    }
    return result;
  }

  private List<File> getDaysDir(File y, File m, Date fromDate, Date toDate) throws ParseException {
    List<File> result = new LinkedList<File>();
    SimpleDateFormat ddf = new SimpleDateFormat("yyyyMMdd");
    for (File d : m.listFiles()) {
      Date dd = ddf.parse(y.getName() + m.getName() + d.getName());
      if (toDate != null && dd.after(ddf.parse(ddf.format(toDate)))) {
        continue;
      }
      if (fromDate != null && dd.before(ddf.parse(ddf.format(fromDate)))) {
        continue;
      }
      result.add(d);
    }
    return result;
  }

  private List<File> getHoursDir(File y, File m, File d, Date fromDate, Date toDate) throws ParseException {
    List<File> result = new LinkedList<File>();
    SimpleDateFormat hdf = new SimpleDateFormat("yyyyMMddHH");

    for (File h : d.listFiles()) {
      Date hd = hdf.parse(y.getName() + m.getName() + d.getName() + h.getName().substring(0, h.getName().indexOf(".")));

      if (toDate != null && hd.after(hdf.parse(hdf.format(toDate)))) {
        continue;
      }
      if (fromDate != null && hd.before(hdf.parse(hdf.format(fromDate)))) {
        continue;
      }
      result.add(h);
    }
    return result;
  }

  private List<File> getFiles(final JournalFilter filter) throws ParseException {

    SimpleDateFormat df = new SimpleDateFormat("yyyyMMddHH");

    Date fromDate = filter == null || filter.getStartDate() == null ? null : df.parse(df.format(filter.getStartDate()));
    Date toDate = filter == null || filter.getEndDate() == null ? null : df.parse(df.format(filter.getEndDate()));

    List<File> results = new LinkedList<File>();

    for(File y : getYearsDir(journalDir, fromDate, toDate)) {
      for(File m : getMonthsDir(y, fromDate, toDate)) {
        for(File d : getDaysDir(y, m ,fromDate, toDate)) {
          for(File h: getHoursDir(y, m, d, fromDate, toDate)) {
            results.add(h);
          }
        }
      }
    }

    if (!results.isEmpty()) {
      Collections.sort(results);
    }
    return results;
  }
}
